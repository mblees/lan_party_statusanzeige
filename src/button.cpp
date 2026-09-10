#include "button.h"

static inline bool rawActive(const Button &b)
{
    const int v = digitalRead(b.pin);
    return b.activeHigh ? (v == HIGH) : (v == LOW);
}

void buttonBegin(Button &b, uint8_t pin, bool activeHigh, uint16_t debounceMs)
{
    b.pin        = pin;
    b.activeHigh = activeHigh;
    b.debounceMs = debounceMs;

    // Der TTP223 treibt den Ausgang aktiv (Push-Pull); der Pull sorgt nur fuer
    // einen definierten Pegel, falls die Signalleitung nicht angeschlossen ist.
    pinMode(pin, activeHigh ? INPUT_PULLDOWN : INPUT_PULLUP);

    b.stable   = rawActive(b);
    b.lastRead = b.stable;
    b.edgeAt   = millis();
}

bool buttonPressed(Button &b)
{
    const bool     raw = rawActive(b);
    const uint32_t now = millis();

    if (raw != b.lastRead)
    {
        b.lastRead = raw;
        b.edgeAt   = now;                 // Rohwert wackelt -> Entprelluhr neu
        return false;
    }

    if (raw != b.stable && (now - b.edgeAt) >= (uint32_t)b.debounceMs)
    {
        b.stable = raw;
        return raw;                       // true nur bei der Flanke -> gedrueckt
    }
    return false;
}
