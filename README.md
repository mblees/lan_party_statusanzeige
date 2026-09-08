# M128_240240 – Rund-Display-Lib für Raspberry Pi Pico 2

Ansteuerung des **M128_240240-RGB-7-V1.0** (1,28″ IPS rund, GC9A01, 240×240, SPI)
an einem Raspberry Pi Pico 2 (RP2350), PlatformIO / Arduino-Framework.

Enthält: Boot-/Intro-Screen mit Logo (Cyan), DVD-Screensaver mit Farbwechsel und
flackerfreiem Vollbild-Framebuffer, BOOTSEL-Taster als Software-Reset.

## Verkabelung

Das Modul hat 7 Pins: `VCC  GND  SCL  SDA  DC  CS  RST`. Kein `BLK` –
die Hintergrundbeleuchtung ist fest an.

| Modul-Pin | → Pico-2-Signal | Pico GPxx | Physischer Pin |
|-----------|-----------------|-----------|----------------|
| **VCC**   | 3V3(OUT)        | –         | **36**         |
| **GND**   | GND             | –         | **38**         |
| **SCL**   | SPI0 SCK        | **GP18**  | **24**         |
| **SDA**   | SPI0 TX (MOSI)  | **GP19**  | **25**         |
| **DC**    | GPIO            | **GP20**  | **26**         |
| **CS**    | SPI0 CS (GPIO)  | **GP17**  | **22**         |
| **RST**   | GPIO            | **GP21**  | **27**         |

```
   Pico 2                         M128_240240 (GC9A01)
 ┌─────────┐
 │ Pin 36  ●─ 3V3(OUT) ───────────●  VCC
 │ Pin 38  ●─ GND  ───────────────●  GND
 │ Pin 24  ●─ GP18 ───────────────●  SCL
 │ Pin 25  ●─ GP19 ───────────────●  SDA
 │ Pin 26  ●─ GP20 ───────────────●  DC
 │ Pin 22  ●─ GP17 ───────────────●  CS
 │ Pin 27  ●─ GP21 ───────────────●  RST
 └─────────┘
```

### Wichtig

- **VCC an 3V3(OUT), nicht an 5 V.** Das Modul hat zwar einen Regler, aber die
  Logikpegel sind 3,3 V und der RP2350 ist nicht 5-V-tolerant.
- SCL/SDA müssen auf demselben SPI-Block liegen (hier SPI0). Werden andere Pins
  gewählt, eine gültige SPI0-Kombination nehmen (SCK: 2/6/18, TX: 3/7/19).
- `DC`, `CS`, `RST` sind normale GPIOs und frei wählbar.
- SPI-Takt ist `TFT_SPI_HZ` = 40 MHz. Bei Bildartefakten über lange Jumper auf
  `24000000` senken; für flüssigeren Screensaver auf `62500000` erhöhen.

### BOOTSEL als Reset

Braucht keine Verkabelung. Im laufenden Programm den BOOTSEL-Taster ~0,1 s
gedrückt halten und **loslassen** → Warm-Reset. Details in
`include/bootsel_reset.h`.

## Pin-Konfiguration ändern

Alle Pins und Parameter stehen in [`include/config.h`](include/config.h):

| Define | Standard | Bedeutung |
|--------|----------|-----------|
| `TFT_SCL_PIN` | `18` | GP-Nummer für SCL/SCK |
| `TFT_SDA_PIN` | `19` | GP-Nummer für SDA/MOSI |
| `TFT_DC_PIN` | `20` | GP-Nummer für DC |
| `TFT_CS_PIN` | `17` | GP-Nummer für CS |
| `TFT_RST_PIN` | `21` | GP-Nummer für RST |
| `TFT_SPI_HZ` | `40000000` | SPI-Takt |
| `TFT_ROTATION` | `0` | Display-Drehung 0..3 |
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
pio run -e pico2 -t upload
pio device monitor -e pico2
```

## Logo neu erzeugen

`logo_binary.png` (schwarz/weiß) → 1-Bit-Bitmap für `src/intro.cpp`
(Zielbreite/-höhe optional, Standard 208):

```bash
python tools/logo_to_bitmap.py logo_binary.png 200 200
```
