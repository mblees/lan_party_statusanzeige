#include "encoder.h"
#include <Arduino.h>

// --------------------------------------------------------------------------
// Quadratur-Dekoder (Zustandstabelle nach Ben Buxton, Public Domain).
// Entprellt die Encoder-Kontakte in Hardware-Naehe: nur saubere
// Gray-Code-Uebergaenge erzeugen einen Schritt, Prellen bleibt folgenlos.
// ENC_FULL_STEP waehlt zwischen Voll- und Halbschritt-Tabelle (siehe encoder.h).
// --------------------------------------------------------------------------
#define DIR_CW  0x10
#define DIR_CCW 0x20

#if ENC_FULL_STEP

#define R_START     0x0
#define R_CW_FINAL  0x1
#define R_CW_BEGIN  0x2
#define R_CW_NEXT   0x3
#define R_CCW_BEGIN 0x4
#define R_CCW_FINAL 0x5
#define R_CCW_NEXT  0x6

static const uint8_t s_ttable[7][4] = {
    // R_START
    {R_START,     R_CW_BEGIN,  R_CCW_BEGIN, R_START},
    // R_CW_FINAL
    {R_CW_NEXT,   R_START,     R_CW_FINAL,  R_START | DIR_CW},
    // R_CW_BEGIN
    {R_CW_NEXT,   R_CW_BEGIN,  R_START,     R_START},
    // R_CW_NEXT
    {R_CW_NEXT,   R_CW_BEGIN,  R_CW_FINAL,  R_START},
    // R_CCW_BEGIN
    {R_CCW_NEXT,  R_START,     R_CCW_BEGIN, R_START},
    // R_CCW_FINAL
    {R_CCW_NEXT,  R_CCW_FINAL, R_START,     R_START | DIR_CCW},
    // R_CCW_NEXT
    {R_CCW_NEXT,  R_CCW_FINAL, R_CCW_BEGIN, R_START},
};

#else // Halbschritt: ein Schritt an beiden Rastpositionen (00 und 11)

#define R_START       0x0
#define R_CCW_BEGIN   0x1
#define R_CW_BEGIN    0x2
#define R_START_M     0x3
#define R_CW_BEGIN_M  0x4
#define R_CCW_BEGIN_M 0x5

static const uint8_t s_ttable[6][4] = {
    // R_START (Rastposition 11)
    {R_START_M,            R_CW_BEGIN,    R_CCW_BEGIN,   R_START},
    // R_CCW_BEGIN
    {R_START_M | DIR_CCW,  R_START,       R_CCW_BEGIN,   R_START},
    // R_CW_BEGIN
    {R_START_M | DIR_CW,   R_CW_BEGIN,    R_START,       R_START},
    // R_START_M (Rastposition 00)
    {R_START_M,            R_CCW_BEGIN_M, R_CW_BEGIN_M,  R_START},
    // R_CW_BEGIN_M
    {R_START_M,            R_START_M,     R_CW_BEGIN_M,  R_START | DIR_CW},
    // R_CCW_BEGIN_M
    {R_START_M,            R_CCW_BEGIN_M, R_START_M,     R_START | DIR_CCW},
};

#endif

static int8_t  s_detents = 0;    // aufsummierte Rasten, von encoderRead() geleert
static uint8_t s_state   = R_START;

// Taster
static bool     s_swStable   = false;
static bool     s_swLastRead = false;
static uint32_t s_swEdgeAt   = 0;

static inline bool swRawActive()
{
#if ENC_SW_ACTIVE_HIGH
    return digitalRead(ENC_SW_PIN) == HIGH;
#else
    return digitalRead(ENC_SW_PIN) == LOW;
#endif
}

static void encoderPoll()
{
    const uint8_t pinState = (uint8_t)((digitalRead(ENC_DT_PIN) << 1) |
                                       digitalRead(ENC_CLK_PIN));
    s_state = s_ttable[s_state & 0x0F][pinState];

    switch (s_state & 0x30)
    {
    case DIR_CW:  if (s_detents < 100) s_detents++; break;
    case DIR_CCW: if (s_detents > -100) s_detents--; break;
    default: break;
    }
}

void encoderBegin()
{
    // KY-040-Platine hat Pull-ups nach +; der interne Pull-up schadet nicht
    // und haelt die Leitungen definiert, falls das Modul nicht bestueckt ist.
    pinMode(ENC_CLK_PIN, INPUT_PULLUP);
    pinMode(ENC_DT_PIN, INPUT_PULLUP);
#if ENC_SW_ACTIVE_HIGH
    pinMode(ENC_SW_PIN, INPUT_PULLDOWN);
#else
    pinMode(ENC_SW_PIN, INPUT_PULLUP);
#endif

    s_state   = R_START;
    s_detents = 0;

    s_swStable   = swRawActive();
    s_swLastRead = s_swStable;
    s_swEdgeAt   = millis();
}

int encoderRead()
{
    encoderPoll();
    const int d = s_detents;
    s_detents = 0;
    return d;
}

bool encoderButtonPressed()
{
    const bool     raw = swRawActive();
    const uint32_t now = millis();

    if (raw != s_swLastRead)
    {
        s_swLastRead = raw;
        s_swEdgeAt   = now;
        return false;
    }

    if (raw != s_swStable && (now - s_swEdgeAt) >= (uint32_t)ENC_SW_DEBOUNCE_MS)
    {
        s_swStable = raw;
        return raw;                 // true nur bei der Flanke -> gedrueckt
    }
    return false;
}
