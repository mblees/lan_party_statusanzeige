# M128_240240 – Rund-Display-Lib für Raspberry Pi Pico 2

Ansteuerung des **M128_240240-RGB-7-V1.0** (1,28″ IPS rund, GC9A01, 240×240, SPI)
an einem Raspberry Pi Pico 2 (RP2350), PlatformIO / Arduino-Framework.

Enthält: Boot-/Intro-Screen mit Logo (Cyan), DVD-Screensaver mit Farbwechsel und
flackerfreiem Vollbild-Framebuffer, BOOTSEL-Taster als Software-Reset.

## Verkabelung

Das Modul hat 7 Pins: `VCC  GND  SCL  SDA  DC  CS  RST`. Kein `BLK` –
die Hintergrundbeleuchtung ist fest an.

Alle Signalleitungen liegen auf der **linken Pico-2-Seite** (Boardpin 1–20),
passend zum Gehäuse-Einbau. Nur `3V3(OUT)` gibt es ausschließlich rechts
(Pin 36); `GND` lässt sich links abgreifen (Pin 3 oder 8).

| Modul-Pin | → Pico-2-Signal | Pico GPxx | Physischer Pin |
|-----------|-----------------|-----------|----------------|
| **VCC**   | 3V3(OUT)        | –         | **36** (rechts) |
| **GND**   | GND             | –         | **8**          |
| **SCL**   | SPI0 SCK        | **GP2**   | **4**          |
| **SDA**   | SPI0 TX (MOSI)  | **GP3**   | **5**          |
| **DC**    | GPIO            | **GP4**   | **6**          |
| **CS**    | SPI0 CS (GPIO)  | **GP1**   | **2**          |
| **RST**   | GPIO            | **GP5**   | **7**          |

```
   Pico 2                         M128_240240 (GC9A01)
 ┌─────────┐
 │ Pin 2   ●─ GP1  ───────────────●  CS
 │ Pin 4   ●─ GP2  ───────────────●  SCL
 │ Pin 5   ●─ GP3  ───────────────●  SDA
 │ Pin 6   ●─ GP4  ───────────────●  DC
 │ Pin 7   ●─ GP5  ───────────────●  RST
 │ Pin 8   ●─ GND  ───────────────●  GND
 │ Pin 36  ●─ 3V3(OUT) ───────────●  VCC   (rechte Seite)
 └─────────┘
```

### Wichtig

- **VCC an 3V3(OUT), nicht an 5 V.** Das Modul hat zwar einen Regler, aber die
  Logikpegel sind 3,3 V und der RP2350 ist nicht 5-V-tolerant.
- SCL/SDA müssen auf demselben SPI-Block liegen (hier SPI0). Werden andere Pins
  gewählt, eine gültige SPI0-Kombination nehmen (SCK: GP2/GP6/GP18,
  TX: GP3/GP7/GP19).
- `DC`, `CS`, `RST` sind normale GPIOs und frei wählbar.
- SPI-Takt ist `TFT_SPI_HZ` = 40 MHz. Bei Bildartefakten über lange Jumper auf
  `24000000` senken; für flüssigeren Screensaver auf `62500000` erhöhen.

### BOOTSEL als Reset

Braucht keine Verkabelung. Im laufenden Programm den BOOTSEL-Taster ~0,1 s
gedrückt halten und **loslassen** → Warm-Reset. Details in
`include/bootsel_reset.h`.

### TTP223-Touch-Taster (2 Stück)

Zwei kapazitive TTP223-Sensoren bedienen das Display:

| Funktion | Modul-Pin | → Pico GPxx | Physischer Pin |
|----------|-----------|-------------|----------------|
| **Bild weiter** | SIG/OUT | **GP6** | **9** |
| **Helligkeitsstufe weiter** | SIG/OUT | **GP7** | **10** |

`VCC` beider Module an `3V3` (Pin 36, rechts), `GND` an `GND` (Pin 8, links).

```
   Pico 2                    TTP223 #1 (Bild)      TTP223 #2 (Helligkeit)
 ┌─────────┐
 │ Pin 8   ●─ GND ───────────●  GND ───────────────●  GND
 │ Pin 9   ●─ GP6 ───────────●  SIG
 │ Pin 10  ●─ GP7 ──────────────────────────────── ●  SIG
 │ Pin 36  ●─ 3V3 ───────────●  VCC ───────────────●  VCC   (rechte Seite)
 └─────────┘
```

- **Bild-Touch:** schaltet auf das nächste PNG weiter.
- **Helligkeits-Touch:** schaltet die Helligkeit stufenweise weiter –
  `0 % → 25 % → 50 % → 75 % → 100 % → 0 % …` (`BRIGHTNESS_STEPS_PCT`).
  Bei **0 %** geht das Display in den Sleep-Modus (`DISPOFF` + `SLPIN`), jede
  andere Stufe weckt es wieder. Bei Software-Dimmung folgt die Anzeige sofort
  (nur der gepufferte Frame wird neu ausgegeben, kein erneutes PNG-Dekodieren).

Standardmäßig arbeitet der TTP223 nicht-rastend und gibt bei Berührung `HIGH`
aus. Rastenden bzw. active-low-Betrieb stellt man am Modul über die Lötbrücken
`A`/`B` ein und passt dann `TTP223_ACTIVE_HIGH` in
[`include/config.h`](include/config.h) an (`TTP223_DEBOUNCE_MS` ebenfalls dort).

### Helligkeit

**Zwei Betriebsarten** (Umschaltung über `TFT_BL_PIN` in
[`include/config.h`](include/config.h)):

- **`TFT_BL_PIN = -1` (Standard):** Das 7-polige Display-Modul hat keinen
  BLK-Pin, die Hintergrundbeleuchtung ist fest an. Die Helligkeit wird per
  **Software-Dimmung der Bildpixel** nachgebildet. Das Maximum ist damit das
  Bild selbst – **heller als die Vorlage geht es nicht**, und der Sleep-Modus
  lässt die Beleuchtung an (nur der Panel-Inhalt wird abgeschaltet).
- **`TFT_BL_PIN >= 0`:** Backlight-Zuleitung des Moduls auftrennen und über
  einen Transistor/MOSFET an diesen PWM-GPIO legen. Dann ist die Helligkeit
  **echt** (bis zum physikalischen Maximum des Displays), die Pixel werden
  nicht mehr gedimmt (kein Framebuffer-Blit nötig), und der Sleep-Modus
  schaltet die Beleuchtung wirklich aus.

## Pin-Konfiguration ändern

Alle Pins und Parameter stehen in [`include/config.h`](include/config.h):

| Define | Standard | Bedeutung |
|--------|----------|-----------|
| `TFT_SCL_PIN` | `2` | GP-Nummer für SCL/SCK (SPI0 SCK: GP2/GP6/GP18) |
| `TFT_SDA_PIN` | `3` | GP-Nummer für SDA/MOSI (SPI0 TX: GP3/GP7/GP19) |
| `TFT_DC_PIN` | `4` | GP-Nummer für DC |
| `TFT_CS_PIN` | `1` | GP-Nummer für CS |
| `TFT_RST_PIN` | `5` | GP-Nummer für RST |
| `TFT_SPI_HZ` | `40000000` | SPI-Takt |
| `TFT_ROTATION` | `0` | Display-Drehung 0..3 |
| `TOUCH_IMAGE_PIN` | `6` | GP-Nummer des TTP223 „Bild weiter" |
| `TOUCH_BRIGHT_PIN` | `7` | GP-Nummer des TTP223 „Helligkeitsstufe weiter" |
| `TTP223_ACTIVE_HIGH` | `1` | `1` = Berührung liefert HIGH (TTP223-Standard) |
| `TTP223_DEBOUNCE_MS` | `30` | Entprellzeit der Touch-Taster [ms] |
| `TFT_BL_PIN` | `-1` | GP-Nummer des Backlight-PWM (Hardware-Dimming); `-1` = Software-Dimmung |
| `TFT_BL_PWM_HZ` | `1000` | PWM-Frequenz der Beleuchtung [Hz] |
| `TFT_BL_ACTIVE_HIGH` | `1` | `1` = hoher Duty = hell |
| `BRIGHTNESS_STEPS_PCT` | `{0,25,50,75,100}` | Helligkeitsstufen in %, der Reihe nach durchgeschaltet |
| `BRIGHTNESS_START_IDX` | `3` | Index der Startstufe (hier 75 %) |
| `INTRO_DURATION_MS` | `3000` | Anzeigedauer des Logos |
| `INTRO_LOGO_Y_OFFSET` | `10` | Logo-Versatz nach unten [px] |
| `TFT_TEXT_SIZE` | `2` | Textgröße der Serial-Ausgabe (1 = 6×8 px/Zeichen) |
| `TFT_TEXT_RADIUS` | `117` | sichtbarer Radius für den Textumbruch [px] |
| `TFT_TEXT_FG` / `TFT_TEXT_BG` | `0xFFFF` / `0x0000` | Text-/Hintergrundfarbe (RGB565) |
| `SERIAL_LINE_MAX` | `256` | max. Zeichen pro empfangener Zeile |

## Text über Serial ausgeben

Nach dem Intro läuft `loop()` als Serial-Terminal: Jede mit `\n` abgeschlossene
Zeile wird zentriert im sichtbaren Kreis ausgegeben. Der Text wird an
Wortgrenzen umgebrochen (zu lange Wörter notfalls hart), sodass **kein Zeichen**
am Kreisrand angeschnitten wird; was nicht mehr passt, entfällt. Eine leere
Zeile löscht das Display.

```bash
pio device monitor -e pico2
# dann Text eintippen und Enter drücken
```

## Bauen & Flashen

```bash
pio run -e pico2 -t upload      # Firmware
pio run -e pico2 -t uploadfs    # Dateisystem (Bilder in data/)
pio device monitor -e pico2
```

## PNG-Bilder anzeigen (LittleFS)

Mehrfarbige Grafiken werden als PNG im LittleFS-Dateisystem des Pico 2
abgelegt – getrennt von der Firmware, also austauschbar ohne Neucompilieren.

1. PNG(s) in den Ordner [`data/`](data/) legen (ideal ≤ 240×240 px). Optional
   aufbereiten – zoomt automatisch so weit hinein, dass das runde Display
   randlos gefüllt ist (Überstand wird zentriert abgeschnitten), legt eine
   weiche Kreismaske an:

   ```bash
   python tools/prepare_image.py rohbild.png smoking
   ```

   `--fit` passt stattdessen das ganze Bild ein und behält dabei einen bewusst
   angelegten Rand (z. B. Visio-Export mit Abstand). `--no-trim` behält den
   transparenten Rand auch im Cover-Modus, `--no-circle` lässt das Ergebnis
   rechteckig.

2. Dateisystem flashen:

   ```bash
   pio run -e pico2 -t uploadfs
   ```

3. Der Pico zeigt nach dem Intro das erste `*.png` aus dem Wurzelverzeichnis;
   jede Berührung des TTP223-Touch-Tasters schaltet auf das nächste weiter
   (siehe [`src/main.cpp`](src/main.cpp)).

Im eigenen Code:

```cpp
#include "image.h"

imageBegin();                                  // in setup(), vor bootselResetBegin()
imageShowPng(tft, "/smoking.png", GC9A01A_BLACK);
```

`imageShowPng()` zentriert das Bild (auch bei abweichender Größe), verrechnet
Transparenz gegen die angegebene Hintergrundfarbe und schneidet Überbreite am
Displayrand ab. Skaliert wird nicht – dafür `tools/prepare_image.py` nutzen.
Details und Formatvergleich in [`data/README.md`](data/README.md).

Einfarbige Logos/Icons bleiben als 1-Bit-Bitmap effizienter – siehe unten.

## Logo neu erzeugen

`logo_binary.png` (schwarz/weiß) → 1-Bit-Bitmap für `src/intro.cpp`
(Zielbreite/-höhe optional, Standard 208):

```bash
python tools/logo_to_bitmap.py logo_binary.png 200 200
```
