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

// ---- TTP223 kapazitive Touch-Taster (2 Stueck) ----
// Modul je:  VCC -> 3V3 (Pin 36), GND -> GND (Pin 38), SIG/OUT -> GPxx.
// TTP223-Standard: nicht-rastend, Ausgang bei Beruehrung HIGH.
#define TTP223_ACTIVE_HIGH  1       // 1 = Beruehrung liefert HIGH
#define TTP223_DEBOUNCE_MS  30      // Entprellzeit [ms]

#define TOUCH_IMAGE_PIN     16      // "naechstes Bild"        -> GP16 (Pin 21)
#define TOUCH_BRIGHT_PIN    10      // "Helligkeitsstufe weiter" -> GP10 (Pin 14)

// ---- Helligkeit ----
// Zwei Betriebsarten, per TFT_BL_PIN gewaehlt:
//
//   TFT_BL_PIN < 0  (Standard): Das 7-polige Display-Modul hat keinen BLK-Pin.
//     "Helligkeit" wird per Software-Dimmung der Bildpixel nachgebildet
//     (0 = schwarz, 255 = Bild unveraendert). Das Maximum ist damit das Bild
//     selbst - heller als die Vorlage geht nicht, und die (fest verdrahtete)
//     Hintergrundbeleuchtung bleibt immer an.
//
//   TFT_BL_PIN >= 0: Backlight-Zuleitung des Moduls auftrennen und ueber einen
//     Transistor/MOSFET an diesen PWM-GPIO legen. Dann ist die Helligkeit echt
//     (bis zum physikalischen Maximum des Displays), die Pixel werden nicht
//     mehr gedimmt und der Sleep-Modus schaltet die Beleuchtung wirklich aus.
#define TFT_BL_PIN         -1       // GPxx des Backlight-PWM, oder -1 = nicht verdrahtet
#define TFT_BL_PWM_HZ      1000     // PWM-Frequenz der Beleuchtung [Hz]
#define TFT_BL_ACTIVE_HIGH 1        // 1 = HIGH/hoher Duty = hell

// Helligkeitsstufen in Prozent. Der Bright-Touch schaltet der Reihe nach
// weiter; nach der letzten Stufe geht es zurueck auf die erste.
// 0 % = Display aus (Sleep-Modus); jede andere Stufe weckt es wieder.
#define BRIGHTNESS_STEPS_PCT { 0, 25, 50, 75, 100 }
#define BRIGHTNESS_START_IDX 3       // Startstufe (Index in obige Liste -> 75 %)

// ---- Boot-/Intro-Screen ----
#define INTRO_DURATION_MS   1000    // Anzeigedauer des Logos nach dem Boot [ms]
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