#ifndef CIRCLETEXT_H
#define CIRCLETEXT_H

#include <Adafruit_GC9A01A.h>

// Zeigt text auf dem runden Display an. Der Text wird an Wortgrenzen (lange
// Woerter notfalls hart) so umgebrochen und zentriert, dass jedes Zeichen
// vollstaendig innerhalb des Kreises mit Radius TFT_TEXT_RADIUS liegt - kein
// Zeichen wird am Rand angeschnitten. Zeichen, die auch bei maximaler
// Zeilenzahl nicht mehr passen, werden weggelassen.
//
// Das Display wird vorher komplett mit TFT_TEXT_BG geloescht. Ein leerer oder
// NULL-Text loescht also nur den Bildschirm.
void circleTextShow(Adafruit_GC9A01A &tft, const char *text);

#endif // CIRCLETEXT_H
