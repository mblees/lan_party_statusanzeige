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

// Alle Signale liegen auf der linken Pico-2-Seite (Boardpin 1..20), damit die
// Verkabelung zum Gehaeuse-Einbau passt. SCL/SDA sind eine gueltige
// SPI0-Kombination (SCK: GP2/GP6, TX: GP3/GP7). Modul-Pin -> Pico:
#define TFT_SCL_PIN      2          // SCL/SCK  -> GP2 (Pin 4)
#define TFT_SDA_PIN      3          // SDA/MOSI -> GP3 (Pin 5)
#define TFT_DC_PIN       4          // DC       -> GP4 (Pin 6)
#define TFT_CS_PIN       1          // CS       -> GP1 (Pin 2)
#define TFT_RST_PIN      5          // RST      -> GP5 (Pin 7)
// VCC -> 3V3 (Pin 36, rechts - einzige 3V3-Quelle), GND -> GND (z. B. Pin 3
// oder 8, links). Kein BLK-Pin (Backlight fest an).

// ---- TTP223 kapazitive Touch-Taster (2 Stueck) ----
// Modul je:  VCC -> 3V3 (Pin 36), GND -> GND (Pin 8), SIG/OUT -> GPxx (links).
// TTP223-Standard: nicht-rastend, Ausgang bei Beruehrung HIGH.
#define TTP223_ACTIVE_HIGH  1       // 1 = Beruehrung liefert HIGH
#define TTP223_DEBOUNCE_MS  30      // Entprellzeit [ms]

#define TOUCH_IMAGE_PIN     6       // "naechstes Bild"          -> GP6 (Pin 9)
#define TOUCH_BRIGHT_PIN    7       // "Helligkeitsstufe weiter"  -> GP7 (Pin 10)

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
#define TFT_BL_PIN         -1       // GPxx des Backlight-PWM (frei links: GP8/Pin11,
                                    // GP9/Pin12), oder -1 = nicht verdrahtet
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