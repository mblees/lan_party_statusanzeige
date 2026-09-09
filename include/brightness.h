#ifndef BRIGHTNESS_H
#define BRIGHTNESS_H

#include <stdint.h>
#include "config.h"

// Zentrale Helligkeitssteuerung (0..255). Je nach TFT_BL_PIN (siehe config.h):
//   < 0  : Software-Dimmung der Bildpixel (imageSetBrightness()).
//   >= 0 : echte PWM auf der Backlight-Leitung.

#ifndef TFT_BL_PIN
#define TFT_BL_PIN -1
#endif
#ifndef TFT_BL_PWM_HZ
#define TFT_BL_PWM_HZ 1000
#endif
#ifndef TFT_BL_ACTIVE_HIGH
#define TFT_BL_ACTIVE_HIGH 1
#endif

// Einmal in setup() aufrufen (nach tft.begin()). Richtet ggf. die Backlight-PWM
// ein und wendet BRIGHTNESS_DEFAULT an.
void brightnessBegin();

// Helligkeit 0..255 setzen.
void brightnessSet(uint8_t level);

// Aktuell gesetzte Helligkeit.
uint8_t brightnessGet();

// true, wenn eine Aenderung ein Neuzeichnen des Bildes erfordert
// (Software-Dimmung). Bei Hardware-PWM false - die Anpassung ist sofort sichtbar.
bool brightnessNeedsRedraw();

// Beleuchtung fuer den Sleep-Modus ausschalten / wiederherstellen. Nur bei
// Hardware-PWM wirksam; die gesetzte Helligkeit bleibt erhalten.
void brightnessSleep(bool sleeping);

#endif // BRIGHTNESS_H
