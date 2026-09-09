#include "main.h"
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_GC9A01A.h>
#include "bootsel_reset.h"
#include "intro.h"
#include "circletext.h"
#include "image.h"
#include "touch.h"

Adafruit_GC9A01A tft(TFT_CS_PIN, TFT_DC_PIN, TFT_RST_PIN); // Hardware-SPI (SPI0)

// ---- Alle PNGs aus dem Dateisystem der Reihe nach zeigen ----
// Weitergeschaltet wird per TTP223-Touch-Taster (touch.h).

static bool     s_haveFs   = false;
static size_t   s_imgIndex = 0;

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

    SPI.setSCK(TFT_SCL_PIN);
    SPI.setTX(TFT_SDA_PIN);
    tft.begin(TFT_SPI_HZ);
    tft.setRotation(TFT_ROTATION);
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
    if (s_haveFs && touchPressed())
    {
        showNextImage();
    }
}
