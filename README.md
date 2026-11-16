# LAN Party Status Display

[![Build](https://github.com/mblees/lan_party_statusanzeige/actions/workflows/build.yml/badge.svg)](https://github.com/mblees/lan_party_statusanzeige/actions/workflows/build.yml)
[![License: MIT](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)

A small desk sign for LAN parties: a 1.28″ round display in a 3D-printed case
that shows where I am while I'm away from my seat — sleeping, on a break, out
for food. Two capacitive touch pads switch the status image and the brightness.

<!-- TODO: photo of the finished device in its case -->

<p align="center">
  <img src="data/sleeping.png" width="180" alt="Status image 'sleeping' as shown on the round display">
</p>

## Features

- **Status images from flash:** PNGs live in a LittleFS partition and are
  flashed separately from the firmware, so new statuses need no recompile.
- **Touch controls:** one TTP223 pad cycles the status image, the other steps
  the brightness `0 → 25 → 50 → 75 → 100 %`. 0 % puts the panel to sleep.
- **Status text over USB:** every line sent over the serial port is shown as
  text, word-wrapped so that no character is clipped by the round edge.
- **Boot logo** rendered from a 1-bit bitmap.
- **BOOT button as reset:** the on-board BOOT button works as a reset button
  while the firmware is running.

## Hardware

| Part | Notes |
|---|---|
| ESP32-C3 Super Mini | RISC-V MCU, native USB-C (serial + flashing) |
| GC9A01 round TFT, 1.28″, 240×240 (module `M128_240240-RGB-7-V1.0`) | 7-pin SPI module, backlight hard-wired on |
| 2 × TTP223 capacitive touch module | default mode: momentary, active high |
| 3D-printed case | |

### Wiring

All pins are set in [`include/config.h`](include/config.h).

| Signal | ESP32-C3 GPIO |
|---|---|
| Display SCL / SCK | GPIO1 |
| Display SDA / MOSI | GPIO3 |
| Display DC | GPIO4 |
| Display RST | GPIO5 |
| Display CS | GPIO10 |
| Touch "next image" (SIG) | GPIO6 |
| Touch "brightness" (SIG) | GPIO7 |
| VCC (display + both touch modules) | 3V3 |
| GND | GND |

GPIO8/9 (strapping pins, GPIO9 = BOOT button) and GPIO20/21 (USB) are kept free on
purpose. Pinout reference: [`docs/esp32_c3_supermini_pinout.jpeg`](docs/esp32_c3_supermini_pinout.jpeg).

## Build and flash

Requires [PlatformIO](https://platformio.org/).

```bash
pio run -t upload       # firmware
pio run -t uploadfs     # LittleFS image with the status pictures from data/
pio device monitor      # serial console
```

## Adding a status image

```bash
python tools/prepare_image.py my_photo.jpg smoking   # -> data/smoking.png, 240x240, round mask
pio run -t uploadfs
```

The script needs Pillow (`pip install Pillow`). See [`data/README.md`](data/README.md) for
the supported formats and options.

## Showing text

Open the serial monitor and type a line, e.g. `Back in 10 min`. It replaces the
current image until the image pad is touched again; an empty line clears the screen.
From a script:

```bash
echo "Getting pizza" > /dev/ttyACM0     # Linux; on Windows use the COM port
```

## Design notes

Some of the problems worth noting:

- **Brightness without a backlight pin.** The 7-pin display module has no
  `BLK` line, so brightness is emulated by scaling every RGB565 pixel. To make
  that fast, every frame is kept in a 115 KB full-screen framebuffer, so a
  brightness step is only a dimmed copy to the panel instead of a new PNG decode.
  If the backlight is wired through a MOSFET, setting `TFT_BL_PIN` switches to real PWM dimming.
- **Text on a round screen.** [`circletext.cpp`](src/circletext.cpp) computes how
  many characters fit on each line of the circle, then looks for the smallest
  line count that fits the text, which gives the best vertical centring. No
  character ever crosses the visible edge.
- **Reset on release, not on press.** GPIO9 is a strapping pin: if it is low
  during reset, the ROM boots into download mode. The reset therefore fires
  when the BOOT button is *released* ([`bootsel_reset.cpp`](src/bootsel_reset.cpp)).

## Project structure

```
include/config.h        pins, timings, brightness steps, text settings
src/main.cpp            setup/loop, touch handling, sleep, serial text
src/image.cpp           PNG decoding (PNGdec) from LittleFS, framebuffer, dimming
src/circletext.cpp      circle-aware text layout
src/brightness.cpp      software dimming or backlight PWM
src/button.cpp          debounced touch/button input
src/bootsel_reset.cpp   BOOT button as reset
src/intro.cpp           boot logo bitmap
data/                   status images (LittleFS)
graphics/               source images for data/
tools/                  image and logo conversion scripts (Python + Pillow)
```

## License

[MIT](LICENSE)
