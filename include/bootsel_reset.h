#ifndef BOOTSEL_RESET_H
#define BOOTSEL_RESET_H

#include "config.h"

// Abtastintervall und Mindest-Druckdauer koennen in config.h ueberschrieben
// werden; hier stehen nur die Defaults.
#ifndef BOOTSEL_RESET_POLL_MS
#define BOOTSEL_RESET_POLL_MS 25u   // wie oft der Timer-Interrupt den Taster prueft [ms]
#endif
#ifndef BOOTSEL_RESET_HOLD_MS
#define BOOTSEL_RESET_HOLD_MS 50u   // so lange muss BOOTSEL gedrueckt sein -> Reset [ms]
#endif

// Richtet einen periodischen Hardware-Timer-Interrupt ein, der den BOOTSEL-
// Taster abtastet. Wird er >= BOOTSEL_RESET_HOLD_MS am Stueck gedrueckt
// gehalten und danach losgelassen, loest der RP2350 einen Warm-Reset aus
// (Verhalten wie RUN -> GND).
//
// Der Reset feuert erst beim Loslassen, damit der Taster beim Neustart offen
// ist - sonst erkennt das Bootrom BOOTSEL als gedrueckt und startet den
// USB-Massenspeicher-Bootloader (RPI-RP2-Laufwerk) statt der Anwendung.
//
// Einmal in setup() aufrufen. Der BOOTSEL-Taster kann keinen echten GPIO-
// Interrupt erzeugen (er haengt an der QSPI-Flash-CS-Leitung), deshalb die
// Loesung ueber einen Timer-Interrupt statt eines Pin-Interrupts.
void bootselResetBegin(void);

// Timer-Interrupt wieder abschalten. Vor Flash-Schreibzugriffen (LittleFS,
// EEPROM-Emulation, OTA) aufrufen - waehrend die Messung CS kurz freigibt,
// darf kein anderer Flash-Zugriff laufen.
void bootselResetEnd(void);

#endif // BOOTSEL_RESET_H
