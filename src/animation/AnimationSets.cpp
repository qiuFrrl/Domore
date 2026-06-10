#include "animation/AnimationSets.h"

namespace robodesk
{
    namespace
    {
        const AnimationId DEFAULT_ANIMATIONS[] = {
            AnimationId::Smile,
            AnimationId::Fast,
            AnimationId::Distracted,
            AnimationId::Buzzing,
            AnimationId::Down,
            AnimationId::Giggle,
            AnimationId::Growing,
            AnimationId::Left,
            AnimationId::Right,
            AnimationId::Playful,
            AnimationId::Splash,
            AnimationId::Spraying,
            AnimationId::Surprised,
            AnimationId::Smirk,
            AnimationId::Shrink,
            AnimationId::Rain,
            AnimationId::Serene,
            AnimationId::Sick,
            AnimationId::Sneeze,
            AnimationId::Sparkle,
        };

        // Set enabled to true when you want these three slots to join the Domore idle pool.
        const TimeAnimationRule TIME_RULES[] = {
            {true, 18, 8, AnimationId::Devil},
            {true, 18, 8, AnimationId::Glowing},
            {true, 18, 8, AnimationId::Menacing},
            {true, 18, 8, AnimationId::Scared},
            {true, 18, 8, AnimationId::Sobbing},
        };

        // Temperature uses Celsius x10, so 20.0 C is 200. Weather codes follow Open-Meteo.
        const WeatherAnimationRule WEATHER_RULES[] = {
            {false, -1000, 200, 0, 999, AnimationId::Sick},
            {false, 330, 1000, 0, 999, AnimationId::Sleepy},
            {false, -1000, 1000, 51, 99, AnimationId::Rain},

        };

        bool timeMatches(const TimeSnapshot &time, const TimeAnimationRule &rule)
        {
            if (!time.valid || !rule.enabled)
            {
                return false;
            }

            if (rule.startHour == rule.endHour)
            {
                return true;
            }

            if (rule.startHour < rule.endHour)
            {
                return time.hour >= rule.startHour && time.hour < rule.endHour;
            }

            return time.hour >= rule.startHour || time.hour < rule.endHour;
        }

        bool weatherMatches(const WeatherData &weather, const WeatherAnimationRule &rule)
        {
            if (!weather.valid || !rule.enabled)
            {
                return false;
            }

            const int16_t temperatureCx10 = static_cast<int16_t>(weather.temperatureC * 10.0f);
            return temperatureCx10 >= rule.minTemperatureCx10 &&
                   temperatureCx10 <= rule.maxTemperatureCx10 &&
                   weather.weatherCode >= rule.minWeatherCode &&
                   weather.weatherCode <= rule.maxWeatherCode;
        }

        void appendUnique(AnimationId animation, AnimationId *out, uint8_t &count, uint8_t maxCount)
        {
            if (out == nullptr || count >= maxCount)
            {
                return;
            }

            for (uint8_t i = 0; i < count; i++)
            {
                if (out[i] == animation)
                {
                    return;
                }
            }

            out[count++] = animation;
        }
    }

    const AnimationId *AnimationSets::defaultAnimations(uint8_t &count)
    {
        count = sizeof(DEFAULT_ANIMATIONS) / sizeof(DEFAULT_ANIMATIONS[0]);
        return DEFAULT_ANIMATIONS;
    }

    void AnimationSets::appendTimeAnimations(const TimeSnapshot &time, AnimationId *out, uint8_t &count, uint8_t maxCount)
    {
        for (const TimeAnimationRule &rule : TIME_RULES)
        {
            if (timeMatches(time, rule))
            {
                appendUnique(rule.animation, out, count, maxCount);
            }
        }
    }

    void AnimationSets::appendWeatherAnimations(const WeatherData &weather, AnimationId *out, uint8_t &count, uint8_t maxCount)
    {
        for (const WeatherAnimationRule &rule : WEATHER_RULES)
        {
            if (weatherMatches(weather, rule))
            {
                appendUnique(rule.animation, out, count, maxCount);
            }
        }
    }

    AnimationId AnimationSets::tapAnimationForCount(uint8_t tapCount)
    {
        if (tapCount >= 12)
        {
            return AnimationId::Swinging;
        }
        if (tapCount >= 9)
        {
            return AnimationId::Fierce;
        }
        if (tapCount >= 8)
        {
            return AnimationId::Furious;
        }
        if (tapCount >= 6)
        {
            return AnimationId::Irritated;
        }
        if (tapCount >= 5)
        {
            return AnimationId::Enraged;
        }
        if (tapCount >= 3)
        {
            return AnimationId::Squint;
        }

        return AnimationId::Drowsy;
    }

    AnimationId AnimationSets::holdAnimation()
    {
        return AnimationId::Shy;
    }

    AnimationId AnimationSets::inactiveAnimation()
    {
        return AnimationId::Sleepy;
    }

    AnimationId AnimationSets::inactiveTapWakeAnimation()
    {
        return AnimationId::Excited;
    }

    AnimationId AnimationSets::inactiveHoldWakeAnimation()
    {
        return AnimationId::Encouragement;
    }

    AnimationId AnimationSets::birthdayFinalAnimation()
    {
        return AnimationId::Love;
    }
}
