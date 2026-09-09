#include "touch.h"
#include <Arduino.h>

static bool     s_stable     = false;   // entprellter Zustand (true = beruehrt)
static bool     s_lastRead   = false;   // letzter Rohwert
static uint32_t s_lastEdgeAt = 0;       // Zeitpunkt der letzten Rohwert-Aenderung

static inline bool touchRawActive() {
#if TOUCH_ACTIVE_HIGH
    return digitalRead(TOUCH_PIN) == HIGH;
#else
    return digitalRead(TOUCH_PIN) == LOW;
#endif
}

void touchBegin() {
    // Der TTP223 treibt den Ausgang aktiv (Push-Pull); der Pull sorgt nur fuer
    // einen definierten Pegel, falls die Signalleitung nicht angeschlossen ist.
#if TOUCH_ACTIVE_HIGH
    pinMode(TOUCH_PIN, INPUT_PULLDOWN);
#else
    pinMode(TOUCH_PIN, INPUT_PULLUP);
#endif
    s_stable     = touchRawActive();
    s_lastRead   = s_stable;
    s_lastEdgeAt = millis();
}

bool touchPressed() {
    const bool     raw = touchRawActive();
    const uint32_t now = millis();

    if (raw != s_lastRead) {
        s_lastRead   = raw;
        s_lastEdgeAt = now;                 // Rohwert wackelt -> Entprelluhr neu
        return false;
    }

    if (raw != s_stable && (now - s_lastEdgeAt) >= (uint32_t)TOUCH_DEBOUNCE_MS) {
        s_stable = raw;
        return raw;                         // true nur bei der Flanke -> beruehrt
    }
    return false;
}
