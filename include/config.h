#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>

#define BAUD_RATE 115200

// ---- BOOTSEL-Taster als Reset (Timer-Interrupt) ----
#define BOOTSEL_RESET_POLL_MS 25     // Abtastintervall des Timer-Interrupts [ms]
#define BOOTSEL_RESET_HOLD_MS 50     // Mindest-Druckdauer bis Warm-Reset [ms]

// ---- Display M128_240240-RGB-7-V1.0 (GC9A01, 1.28" rund, 240x240, SPI) ----
#define TFT_WIDTH        240
#define TFT_HEIGHT       240
#define TFT_ROTATION     0          // 0..3
#define TFT_SPI_HZ       40000000UL // SPI-Takt; bei langen Leitungen ggf. senken

// SPI0 auf dem Pico (GPxx-Nummer, nicht Boardpin). Modul-Pin -> Pico:
#define TFT_SCL_PIN      18         // SCL/SCK  -> GP18 (Pin 24)
#define TFT_SDA_PIN      19         // SDA/MOSI -> GP19 (Pin 25)
#define TFT_DC_PIN       20         // DC       -> GP20 (Pin 26)
#define TFT_CS_PIN       17         // CS       -> GP17 (Pin 22)
#define TFT_RST_PIN      21         // RST      -> GP21 (Pin 27)
// VCC -> 3V3 (Pin 36), GND -> GND. Kein BLK-Pin (Backlight fest an).

// ---- Boot-/Intro-Screen ----
#define INTRO_DURATION_MS   3000    // Anzeigedauer des Logos nach dem Boot [ms]
#define INTRO_LOGO_Y_OFFSET 20      // Logo um so viele Pixel nach unten schieben

// ---- Text-Ausgabe (Serial -> Display) ----
// Ueber den Serial-Bus gesendete Zeilen (mit '\n' abgeschlossen) werden auf dem
// Display ausgegeben. Der Text wird so umgebrochen, dass er komplett innerhalb
// des sichtbaren Kreises liegt; kein Zeichen wird am Kreisrand angeschnitten.
#define TFT_TEXT_SIZE       2       // Adafruit-GFX-Textgroesse (1 = 6x8 px/Zeichen)
#define TFT_TEXT_RADIUS     117     // sichtbarer Radius des runden Displays [px]
#define TFT_TEXT_FG         0xFFFF  // Textfarbe  (RGB565, 0xFFFF = weiss)
#define TFT_TEXT_BG         0x0000  // Hintergrund (RGB565, 0x0000 = schwarz)
#define SERIAL_LINE_MAX     256     // max. Zeichen pro empfangener Zeile

#endif // CONFIG_H