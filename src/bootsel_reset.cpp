#include "bootsel_reset.h"
#include <Arduino.h>
#include <pico/time.h>
#include <pico/stdlib.h>
#include <hardware/watchdog.h>

static repeating_timer_t s_timer;
static volatile uint16_t  s_heldMs  = 0;
static volatile bool      s_running = false;
static volatile bool      s_armed   = false;      // lange genug gedrueckt, wartet auf Loslassen

// Callback des Default-Alarm-Pools: laeuft im Timer-Hardware-Interrupt.
static bool bootselPollIsr(repeating_timer_t *) {
    // BOOTSEL (arduino-pico-Core): legt die Flash-CS-Leitung kurz auf Hi-Z,
    // parkt den zweiten Core (No-op wenn nicht gestartet), sperrt IRQs,
    // misst den Pegel und stellt alles wieder her. Dauer ~50 us.
    if (BOOTSEL) {
        if (s_heldMs < (uint16_t)BOOTSEL_RESET_HOLD_MS) {
            s_heldMs += (uint16_t)BOOTSEL_RESET_POLL_MS;
            if (s_heldMs >= (uint16_t)BOOTSEL_RESET_HOLD_MS) {
                s_armed = true;                    // scharf: Reset beim Loslassen
            }
        }
    } else {
        if (s_armed) {
            // Taster ist jetzt losgelassen -> beim Neustart sieht das Bootrom
            // BOOTSEL offen und startet die Anwendung statt des USB-Bootloaders.
            watchdog_reboot(0, 0, 0);              // sofortiger Warm-Reset
            while (true) {
                tight_loop_contents();
            }
        }
        s_heldMs = 0;                              // losgelassen -> Zaehler zurueck
    }
    return true;                                   // Timer weiterlaufen lassen
}

void bootselResetBegin(void) {
    if (s_running) {
        return;
    }
    s_heldMs = 0;
    s_armed  = false;
    // negatives Intervall => konstanter Abstand zwischen den Aufrufen
    s_running = add_repeating_timer_ms(-(int32_t)BOOTSEL_RESET_POLL_MS,
                                       bootselPollIsr, nullptr, &s_timer);
    if (!s_running) {
        Serial.println(F("bootselResetBegin: kein Timer frei"));
    }
}

void bootselResetEnd(void) {
    if (!s_running) {
        return;
    }
    cancel_repeating_timer(&s_timer);
    s_running = false;
}
