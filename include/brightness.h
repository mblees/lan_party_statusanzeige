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
// ein und wendet die Startstufe (BRIGHTNESS_START_IDX) an.
void brightnessBegin();

// Helligkeit direkt als 0..255 setzen.
void brightnessSet(uint8_t level);

// Aktuell gesetzte Helligkeit (0..255).
uint8_t brightnessGet();

// --- Stufenbetrieb (BRIGHTNESS_STEPS_PCT) ---
// Auf die naechste Stufe weiterschalten (nach der letzten wieder auf die erste)
// und anwenden. Liefert die neue Stufe in Prozent (0..100).
uint8_t brightnessNextStep();

// Aktuelle Stufe in Prozent (0..100).
uint8_t brightnessStepPercent();

// true, wenn eine Aenderung ein Neuzeichnen des Bildes erfordert
// (Software-Dimmung). Bei Hardware-PWM false - die Anpassung ist sofort sichtbar.
bool brightnessNeedsRedraw();

// Beleuchtung fuer den Sleep-Modus ausschalten / wiederherstellen. Nur bei
// Hardware-PWM wirksam; die gesetzte Helligkeit bleibt erhalten.
void brightnessSleep(bool sleeping);

#endif // BRIGHTNESS_H
