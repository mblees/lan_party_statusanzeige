#include "brightness.h"
#include "image.h"
#include <Arduino.h>

static const uint8_t s_stepsPct[] = BRIGHTNESS_STEPS_PCT;
static const uint8_t s_stepCount  = sizeof(s_stepsPct) / sizeof(s_stepsPct[0]);
static uint8_t       s_stepIdx    = BRIGHTNESS_START_IDX;

static uint8_t s_level = 0;   // aktuell gesetzte Helligkeit 0..255

static inline uint8_t pctToLevel(uint8_t pct)
{
    return (uint8_t)((pct * 255u + 50u) / 100u);   // gerundet
}

#if TFT_BL_PIN >= 0
static inline void blWrite(uint8_t level)
{
    analogWrite(TFT_BL_PIN, TFT_BL_ACTIVE_HIGH ? level : (uint8_t)(255 - level));
}
#endif

void brightnessBegin()
{
#if TFT_BL_PIN >= 0
    analogWriteFreq(TFT_BL_PWM_HZ);
    analogWriteRange(255);
    pinMode(TFT_BL_PIN, OUTPUT);
    imageSetBrightness(255);        // Pixel nicht zusaetzlich dimmen
#endif
    if (s_stepIdx >= s_stepCount)
        s_stepIdx = 0;
    brightnessSet(pctToLevel(s_stepsPct[s_stepIdx]));
}

void brightnessSet(uint8_t level)
{
    s_level = level;
#if TFT_BL_PIN >= 0
    blWrite(level);
#else
    imageSetBrightness(level);
#endif
}

uint8_t brightnessGet()
{
    return s_level;
}

uint8_t brightnessNextStep()
{
    s_stepIdx = (uint8_t)((s_stepIdx + 1) % s_stepCount);
    const uint8_t pct = s_stepsPct[s_stepIdx];
    brightnessSet(pctToLevel(pct));
    return pct;
}

uint8_t brightnessStepPercent()
{
    return s_stepsPct[s_stepIdx];
}

bool brightnessNeedsRedraw()
{
#if TFT_BL_PIN >= 0
    return false;
#else
    return true;
#endif
}

void brightnessSleep(bool sleeping)
{
#if TFT_BL_PIN >= 0
    if (sleeping)
        blWrite(0);
    else
        blWrite(s_level);
#else
    (void)sleeping;
#endif
}
