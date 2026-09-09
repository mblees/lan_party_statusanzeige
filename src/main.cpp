#include "main.h"
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_GC9A01A.h>
#include "bootsel_reset.h"
#include "intro.h"
#include "circletext.h"
#include "image.h"
#include "touch.h"
#include "encoder.h"
#include "brightness.h"

Adafruit_GC9A01A tft(TFT_CS_PIN, TFT_DC_PIN, TFT_RST_PIN); // Hardware-SPI (SPI0)

// ---- Alle PNGs aus dem Dateisystem der Reihe nach zeigen ----
// TTP223-Touch-Taster : naechstes Bild
// KY-040 drehen        : Helligkeit (Software-Dimmung, siehe image.h)
// KY-040 druecken      : Display-Sleep an/aus

static bool   s_haveFs   = false;
static size_t s_imgIndex = 0;
static bool   s_asleep   = false;

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

// Display schlafen legen / aufwecken. Das Modul hat keinen Backlight-Pin,
// die Beleuchtung bleibt also an - der Panel-Inhalt wird aber abgeschaltet
// und der GC9A01 in den stromsparenden Sleep-Modus versetzt.
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

void setup()
{
    Serial.begin(BAUD_RATE);
    Serial.println("Serial connected.");

    // LittleFS zuerst mounten: schlaegt das Mounten fehl, formatiert LittleFS
    // das Flash - dieser Schreibzugriff soll abgeschlossen sein, bevor der
    // BOOTSEL-Reset-Timer laeuft (der gibt die Flash-CS-Leitung kurz frei).
    s_haveFs = imageBegin();

    bootselResetBegin(); // BOOTSEL-Taster wirkt ab jetzt als Reset
    touchBegin();        // TTP223-Touch-Taster schaltet die Bilder weiter
    encoderBegin();      // KY-040: Helligkeit + Sleep

    SPI.setSCK(TFT_SCL_PIN);
    SPI.setTX(TFT_SDA_PIN);
    tft.begin(TFT_SPI_HZ);
    tft.setRotation(TFT_ROTATION);
    brightnessBegin();   // Backlight-PWM bzw. Software-Dimmung, BRIGHTNESS_DEFAULT
    tft.fillScreen(GC9A01A_BLACK);

    Serial.println("TFT initialised");

    // Boot-/Intro-Screen: Logo statisch fuer INTRO_DURATION_MS anzeigen
    introShow(tft, INTRO_DURATION_MS);

    if (s_haveFs)
    {
        showImage(s_imgIndex); // erstes Bild sofort
    }
    else
    {
        circleTextShow(tft, "LittleFS fehlt - 'pio run -e pico2 -t uploadfs'");
    }
}

void loop()
{
    // --- KY-040 Tastendruck: Display-Sleep umschalten ---
    if (encoderButtonPressed())
        displaySetSleep(!s_asleep);

    // --- KY-040 drehen: Helligkeit anpassen ---
    const int detents = encoderRead();
    if (detents != 0)
    {
        const int oldLevel = brightnessGet();
        int level = oldLevel - detents * BRIGHTNESS_STEP;
        if (level < BRIGHTNESS_MIN) level = BRIGHTNESS_MIN;
        if (level > BRIGHTNESS_MAX) level = BRIGHTNESS_MAX;

        // An der oberen/unteren Grenze aendert Weiterdrehen nichts.
        if (level != oldLevel)
        {
            brightnessSet((uint8_t)level);
            Serial.printf("[brightness] %d\n", level);

            // Bei Software-Dimmung sofort neu ausgeben: imageRefresh() schiebt
            // nur den Framebuffer (gedimmt) auf das Display, ohne das PNG neu
            // zu dekodieren. Bei Hardware-PWM wirkt die Aenderung ohnehin sofort.
            if (!s_asleep && s_haveFs && brightnessNeedsRedraw())
                imageRefresh();
        }
    }

    // --- TTP223: naechstes Bild (im Sleep ignoriert) ---
    if (s_haveFs && !s_asleep && touchPressed())
        showNextImage();
}
