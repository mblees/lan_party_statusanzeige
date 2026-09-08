#include "main.h"
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_GC9A01A.h>
#include "bootsel_reset.h"
#include "intro.h"
#include "circletext.h"

Adafruit_GC9A01A tft(TFT_CS_PIN, TFT_DC_PIN, TFT_RST_PIN); // Hardware-SPI (SPI0)

// Vollbild-Framebuffer: jeder Frame wird komplett im RAM aufgebaut und in
// einem Rutsch geschoben -> kein Flackern durch Loeschen/Neuzeichnen.
static GFXcanvas16 fb(TFT_WIDTH, TFT_HEIGHT);

void setup()
{
    Serial.begin(BAUD_RATE);
    Serial.println("Serial connected.");

    bootselResetBegin(); // BOOTSEL-Taster wirkt ab jetzt als Reset

    SPI.setSCK(TFT_SCL_PIN);
    SPI.setTX(TFT_SDA_PIN);
    tft.begin(TFT_SPI_HZ);
    tft.setRotation(TFT_ROTATION);
    tft.fillScreen(GC9A01A_BLACK);

    Serial.println("TFT initialised");

    // Boot-/Intro-Screen: Logo statisch fuer INTRO_DURATION_MS anzeigen
    introShow(tft, INTRO_DURATION_MS);
}

void loop()
{

}
