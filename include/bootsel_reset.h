#ifndef BOOTSEL_RESET_H
#define BOOTSEL_RESET_H

#include "config.h"

// Abtastintervall und Mindest-Druckdauer koennen in config.h ueberschrieben
// werden; hier stehen nur die Defaults.
#ifndef BOOTSEL_RESET_POLL_MS
#define BOOTSEL_RESET_POLL_MS 25u   // wie oft der Taster geprueft wird [ms]
#endif
#ifndef BOOTSEL_RESET_HOLD_MS
#define BOOTSEL_RESET_HOLD_MS 50u   // so lange muss der Taster gedrueckt sein -> Reset [ms]
#endif

// GPIO des eingebauten BOOT-Tasters (Strapping-Pin, zieht bei Druck nach GND).
#ifndef BOOTSEL_RESET_PIN
#define BOOTSEL_RESET_PIN 9u
#endif

// Richtet eine periodische Abtastung des BOOT-Tasters ein. Wird er
// >= BOOTSEL_RESET_HOLD_MS am Stueck gedrueckt gehalten und danach
// losgelassen, loest das Board einen Warm-Reset aus (ESP.restart()).
//
// Der Reset feuert erst beim Loslassen, damit der Taster beim Neustart offen
// ist - sonst startet das Bootrom den seriellen Download-Modus statt der
// Anwendung. Einmal in setup() aufrufen.
void bootselResetBegin(void);

// Abtastung wieder abschalten.
void bootselResetEnd(void);

#endif // BOOTSEL_RESET_H
