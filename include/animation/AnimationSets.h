#pragma once

#include <Arduino.h>

#include "display/AnimationCatalog.h"
#include "system/TimeManager.h"
#include "system/WeatherManager.h"

namespace robodesk
{
struct TimeAnimationRule
{
    bool enabled;
    uint8_t startHour;
    uint8_t endHour;
    AnimationId animation;
};

struct WeatherAnimationRule
{
    bool enabled;
    int16_t minTemperatureCx10;
    int16_t maxTemperatureCx10;
    uint16_t minWeatherCode;
    uint16_t maxWeatherCode;
    AnimationId animation;
};

namespace AnimationSets
{
const AnimationId *defaultAnimations(uint8_t &count);
void appendTimeAnimations(const TimeSnapshot &time, AnimationId *out, uint8_t &count, uint8_t maxCount);
void appendWeatherAnimations(const WeatherData &weather, AnimationId *out, uint8_t &count, uint8_t maxCount);
AnimationId tapAnimationForCount(uint8_t tapCount);
AnimationId holdAnimation();
AnimationId inactiveAnimation();
AnimationId inactiveTapWakeAnimation();
AnimationId inactiveHoldWakeAnimation();
AnimationId lowBatteryAnimation();
AnimationId birthdayFinalAnimation();
}
}
