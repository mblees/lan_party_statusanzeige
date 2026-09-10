#ifndef BUTTON_H
#define BUTTON_H

#include <Arduino.h>

// Entprellter Momentan-Taster (z. B. TTP223). Mehrfach instanziierbar - je
// Taster eine Button-Struktur anlegen.
struct Button
{
    uint8_t  pin;
    bool     activeHigh;
    uint16_t debounceMs;
    bool     stable;      // entprellter Zustand (true = gedrueckt/beruehrt)
    bool     lastRead;    // letzter Rohwert
    uint32_t edgeAt;      // Zeitpunkt der letzten Rohwert-Aenderung
};

// GPIO einrichten. activeHigh = true: gedrueckt liefert HIGH (TTP223-Standard).
// Einmal in setup() aufrufen.
void buttonBegin(Button &b, uint8_t pin, bool activeHigh, uint16_t debounceMs);

// In loop() aufrufen. Liefert genau einmal true im Moment des Druecks
// (Flanke inaktiv -> aktiv); dauerhaftes Halten loest nur einen Impuls aus.
bool buttonPressed(Button &b);

#endif // BUTTON_H
