#ifndef TOUCH_H
#define TOUCH_H

#include "config.h"

// TTP223 kapazitiver Touch-Taster: bei jeder Beruehrung wird ein Schritt
// weitergeschaltet (naechstes Bild). Das Modul hat 3 Pins:
//
//     VCC -> 3V3 (Pin 36)      GND -> GND (Pin 38)      SIG/OUT -> TOUCH_PIN
//
// TTP223-Standardkonfiguration: nicht-rastend, Ausgang bei Beruehrung HIGH.
// Andere Modi (rastend, active-low) stellt man am Modul ueber die Loetbruecken
// A/B ein; TOUCH_ACTIVE_HIGH in config.h dann entsprechend anpassen.

// Pin und Parameter koennen in config.h ueberschrieben werden; hier die Defaults.
#ifndef TOUCH_PIN
#define TOUCH_PIN 16            // SIG/OUT des TTP223 -> GP16 (Pin 21)
#endif
#ifndef TOUCH_ACTIVE_HIGH
#define TOUCH_ACTIVE_HIGH 1     // 1 = Beruehrung -> HIGH (TTP223-Standard)
#endif
#ifndef TOUCH_DEBOUNCE_MS
#define TOUCH_DEBOUNCE_MS 30    // Entprellzeit [ms] (das Modul entprellt selbst)
#endif

// GPIO fuer das TTP223-Modul einrichten. Einmal in setup() aufrufen.
void touchBegin();

// In loop() aufrufen. Liefert genau einmal true im Moment des Antippens
// (Flanke inaktiv -> aktiv). Dauerhaftes Beruehren loest nur einen Impuls aus.
bool touchPressed();

#endif // TOUCH_H
