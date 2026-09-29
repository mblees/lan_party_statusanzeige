#include "bootsel_reset.h"
#include <Arduino.h>
#include <esp_timer.h>

// ESP32-C3 Super Mini: der eingebaute BOOT-Taster ist ein normaler GPIO
// (zieht bei Druck nach GND, interner Pull-up), daher reicht ein einfacher
// periodischer Software-Timer (esp_timer).

static esp_timer_handle_t s_timer   = nullptr;
static volatile uint16_t  s_heldMs  = 0;
static volatile bool      s_armed   = false;

static void bootselPollCb(void *) {
    if (digitalRead(BOOTSEL_RESET_PIN) == LOW) {   // Taster gedrueckt
        if (s_heldMs < (uint16_t)BOOTSEL_RESET_HOLD_MS) {
            s_heldMs += (uint16_t)BOOTSEL_RESET_POLL_MS;
            if (s_heldMs >= (uint16_t)BOOTSEL_RESET_HOLD_MS) {
                s_armed = true;                    // scharf: Reset beim Loslassen
            }
        }
    } else {
        if (s_armed) {
            // Taster ist jetzt losgelassen -> beim Neustart sieht das Bootrom
            // GPIO9 offen (HIGH) und startet die Anwendung statt des
            // seriellen Download-Modus.
            ESP.restart();
        }
        s_heldMs = 0;                              // losgelassen -> Zaehler zurueck
    }
}

void bootselResetBegin(void) {
    if (s_timer) {
        return;
    }
    pinMode(BOOTSEL_RESET_PIN, INPUT_PULLUP);
    s_heldMs = 0;
    s_armed  = false;

    const esp_timer_create_args_t args = {
        .callback       = &bootselPollCb,
        .arg            = nullptr,
        .dispatch_method = ESP_TIMER_TASK,
        .name           = "bootsel_poll",
    };
    if (esp_timer_create(&args, &s_timer) != ESP_OK) {
        Serial.println(F("bootselResetBegin: esp_timer_create fehlgeschlagen"));
        return;
    }
    esp_timer_start_periodic(s_timer, (uint64_t)BOOTSEL_RESET_POLL_MS * 1000ULL);
}

void bootselResetEnd(void) {
    if (!s_timer) {
        return;
    }
    esp_timer_stop(s_timer);
    esp_timer_delete(s_timer);
    s_timer = nullptr;
}
