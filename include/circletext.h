#ifndef CIRCLETEXT_H
#define CIRCLETEXT_H

#include <Adafruit_GFX.h>

// Zeigt text auf dem runden Display an. Der Text wird an Wortgrenzen (lange
// Woerter notfalls hart) so umgebrochen und zentriert, dass jedes Zeichen
// vollstaendig innerhalb des Kreises mit Radius TFT_TEXT_RADIUS liegt - kein
// Zeichen wird am Rand angeschnitten. Zeichen, die auch bei maximaler
// Zeilenzahl nicht mehr passen, werden weggelassen.
//
// Das Ziel wird vorher komplett mit TFT_TEXT_BG geloescht. Ein leerer oder
// NULL-Text loescht also nur den Bildschirm. gfx ist entweder das Display
// (Adafruit_GC9A01A) oder ein Framebuffer-Canvas (siehe imageShowText()).
void circleTextShow(Adafruit_GFX &gfx, const char *text);

// Wie circleTextShow(), aber Umbruch- und Zentrierkreis sind frei waehlbar
// (Mittelpunkt cx/cy, Radius r in Pixeln). So laesst sich Text z. B. in die
// untere Displayhaelfte legen, waehrend oben eine Grafik steht.
//   clear == true : Ziel zuerst mit TFT_TEXT_BG loeschen (wie circleTextShow())
//   clear == false: bestehenden Inhalt stehen lassen (in denselben Framebuffer
//                   zeichnen wie zuvor eine andere Grafik)
void circleTextShowIn(Adafruit_GFX &gfx, const char *text,
                      float cx, float cy, float r, bool clear = true);

#endif // CIRCLETEXT_H
