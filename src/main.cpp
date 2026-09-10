#include "main.h"
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_GC9A01A.h>
#include "bootsel_reset.h"
#include "intro.h"
#include "circletext.h"
#include "image.h"
#include "button.h"
#include "brightness.h"

Adafruit_GC9A01A tft(TFT_CS_PIN, TFT_DC_PIN, TFT_RST_PIN); // Hardware-SPI (SPI0)

// ---- Bedienung ----
// TTP223 an TOUCH_IMAGE_PIN  : naechstes Bild
// TTP223 an TOUCH_BRIGHT_PIN : naechste Helligkeitsstufe (0/25/50/75/100 %);
//                              0 % legt das Display schlafen, jede andere weckt es

static bool   s_haveFs   = false;
static size_t s_imgIndex = 0;
static bool   s_asleep   = false;

static Button s_btnImage;
static Button s_btnBright;

static void showImage(size_t index)
{
    if (imageCount() == 0)
    {
        circleTextShow(tft, "Keine PNGs im Dateisystem. Bilder nach data/ "
                            "legen und 'pio run -e pico2 -t uploadfs'");
        return;
    }

    const char *path = imageName(index);
    if (!imageShowPng(tft, path, GC9A01A_BLACK))
        circleTextShow(tft, path); // Fehlertext statt Bild
}

static void showNextImage()
{
    if (imageCount() == 0)
    {
        showImage(0); // gibt den Hinweistext aus
        return;
    }

    s_imgIndex = (s_imgIndex + 1) % imageCount();
    showImage(s_imgIndex);
}

// Display schlafen legen / aufwecken. Das Modul hat keinen Backlight-Pin, die
// Beleuchtung bleibt also an (ausser bei Hardware-PWM, siehe brightness) - der
// Panel-Inhalt wird abgeschaltet und der GC9A01 in den Sleep-Modus versetzt.
static void displaySetSleep(bool sleep)
{
    if (sleep == s_asleep)
        return;
    s_asleep = sleep;

    if (sleep)
    {
        tft.sendCommand(GC9A01A_DISPOFF);
        tft.sendCommand(GC9A01A_SLPIN);
        brightnessSleep(true); // Beleuchtung aus (nur bei Hardware-PWM wirksam)
    }
    else
    {
        brightnessSleep(false);
        tft.sendCommand(GC9A01A_SLPOUT);
        delay(120); // Datenblatt: >= 120 ms bis zum naechsten Befehl
        tft.sendCommand(GC9A01A_DISPON);
        if (s_haveFs)
            imageRefresh(); // gepufferten Frame wieder ausgeben
    }
}

// Reaktion auf einen Druck des Helligkeits-Touch.
static void brightnessStepTouched()
{
    const uint8_t pct = brightnessNextStep();
    Serial.printf("[brightness] %u%%\n", pct);

    if (pct == 0)
    {
        displaySetSleep(true);
    }
    else if (s_asleep)
    {
        displaySetSleep(false); // weckt auf und zeichnet mit der neuen Helligkeit
    }
    else if (s_haveFs && brightnessNeedsRedraw())
    {
        imageRefresh();         // Software-Dimmung: sofort neu ausgeben
    }
}

void setup()
{
    Serial.begin(BAUD_RATE);
    Serial.println("Serial connected.");

    // LittleFS zuerst mounten: schlaegt das Mounten fehl, formatiert LittleFS
    // das Flash - dieser Schreibzugriff soll abgeschlossen sein, bevor der
    // BOOTSEL-Reset-Timer laeuft (der gibt die Flash-CS-Leitung kurz frei).
    s_haveFs = imageBegin();

    bootselResetBegin(); // BOOTSEL-Taster wirkt ab jetzt als Reset
    buttonBegin(s_btnImage,  TOUCH_IMAGE_PIN,  TTP223_ACTIVE_HIGH, TTP223_DEBOUNCE_MS);
    buttonBegin(s_btnBright, TOUCH_BRIGHT_PIN, TTP223_ACTIVE_HIGH, TTP223_DEBOUNCE_MS);

    SPI.setSCK(TFT_SCL_PIN);
    SPI.setTX(TFT_SDA_PIN);
    tft.begin(TFT_SPI_HZ);
    tft.setRotation(TFT_ROTATION);
    brightnessBegin();   // Backlight-PWM bzw. Software-Dimmung, Startstufe
    tft.fillScreen(GC9A01A_BLACK);

    Serial.println("TFT initialised");

    // Boot-/Intro-Screen: Logo statisch fuer INTRO_DURATION_MS anzeigen
    introShow(tft, INTRO_DURATION_MS);

    if (s_haveFs)
        showImage(s_imgIndex); // erstes Bild sofort
    else
        circleTextShow(tft, "LittleFS fehlt - 'pio run -e pico2 -t uploadfs'");

    // Startstufe 0 % -> direkt schlafen legen.
    if (brightnessStepPercent() == 0)
        displaySetSleep(true);
}

void loop()
{
    // Beide Taster in jedem Durchlauf abfragen, damit die Entprellung sauber
    // laeuft (auch der Bild-Taster, dessen Wirkung im Sleep entfaellt).
    const bool brightHit = buttonPressed(s_btnBright);
    const bool imageHit  = buttonPressed(s_btnImage);

    if (brightHit)
        brightnessStepTouched();

    if (imageHit && s_haveFs && !s_asleep)
        showNextImage();
}
