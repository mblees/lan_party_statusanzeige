#ifndef ENCODER_H
#define ENCODER_H

#include "config.h"

// KY-040 Dreh-Encoder mit Taster. Modul-Pins:
//
//     + -> 3V3 (Pin 36)   GND -> GND (Pin 38)
//     CLK -> ENC_CLK_PIN   DT -> ENC_DT_PIN   SW -> ENC_SW_PIN
//
// Der KY-040 rastet mechanisch; eine Raste entspricht einer vollen
// Quadratur-Periode. encoderRead() liefert die Zahl der Rasten seit dem
// letzten Aufruf (positiv = im Uhrzeigersinn).

#ifndef ENC_CLK_PIN
#define ENC_CLK_PIN 10
#endif
#ifndef ENC_DT_PIN
#define ENC_DT_PIN 11
#endif
#ifndef ENC_SW_PIN
#define ENC_SW_PIN 12
#endif
#ifndef ENC_SW_ACTIVE_HIGH
#define ENC_SW_ACTIVE_HIGH 0
#endif
#ifndef ENC_SW_DEBOUNCE_MS
#define ENC_SW_DEBOUNCE_MS 40
#endif

// Schritte pro Raste. Viele KY-040 rasten alle halbe Quadratur-Periode; mit
// dem Vollschritt-Dekoder zaehlt dann nur jede zweite Raste (1). Der
// Halbschritt-Dekoder (0) liefert bei solchen Encodern einen Schritt pro Raste.
// Zaehlt es danach doppelt, wieder auf 1 stellen.
#ifndef ENC_FULL_STEP
#define ENC_FULL_STEP 0
#endif

// GPIOs einrichten. Einmal in setup() aufrufen.
void encoderBegin();

// In loop() haeufig aufrufen (pollt die Quadratur-Signale). Liefert die
// aufsummierten Rasten seit dem letzten Aufruf und setzt den Zaehler zurueck:
//   > 0  im Uhrzeigersinn gedreht
//   < 0  gegen den Uhrzeigersinn
//   0    keine Bewegung
int encoderRead();

// In loop() aufrufen. Liefert genau einmal true im Moment des Tastendrucks.
bool encoderButtonPressed();

#endif // ENCODER_H
