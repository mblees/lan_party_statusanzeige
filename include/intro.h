#ifndef INTRO_H
#define INTRO_H

#include <Adafruit_GC9A01A.h>

// Boot-/Intro-Screen: zeigt das Logo statisch (keine Animation) fuer
// durationMs Millisekunden an und loescht das Display danach wieder.
// Blockierend - erst nach Ablauf der Zeit kehrt die Funktion zurueck.
void introShow(Adafruit_GC9A01A &tft, uint16_t durationMs);

// Nur das Logo zeichnen (ohne delay/clear), z. B. zum Kombinieren.
void introDrawLogo(Adafruit_GC9A01A &tft);

// Rohdaten: 1 Bit/Pixel, row-major, MSB zuerst (Adafruit-GFX-Bitmap).
#define INTRO_LOGO_WIDTH  200
#define INTRO_LOGO_HEIGHT 152
extern const unsigned char INTRO_LOGO_BMP[3800];

#endif // INTRO_H
