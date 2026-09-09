#include "brightness.h"
#include "image.h"
#include <Arduino.h>

static uint8_t s_level = BRIGHTNESS_DEFAULT;

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
    brightnessSet(s_level);
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
