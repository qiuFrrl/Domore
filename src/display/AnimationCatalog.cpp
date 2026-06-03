#include "display/AnimationCatalog.h"

#include <string.h>

#include "assets/animation/allAnim.h"

namespace robodesk
{
    namespace
    {
        const AnimationClip CLIPS[] = {
            {"Blank", 128, 64, BlankallArray_LEN, BlankallArray, 100},
            {"Buzzing", 128, 64, BuzzingallArray_LEN, BuzzingallArray, 90},
            {"Devil", 128, 64, DevilallArray_LEN, DevilallArray, 90},
            {"Distracted", 128, 64, DistractedallArray_LEN, DistractedallArray, 90},
            {"Down", 128, 64, DownallArray_LEN, DownallArray, 90},
            {"Drowsy", 128, 64, DrowsyallArray_LEN, DrowsyallArray, 90},
            {"Encouragement", 128, 64, EncouragementallArray_LEN, EncouragementallArray, 90},
            {"Enraged", 128, 64, EnragedallArray_LEN, EnragedallArray, 90},
            {"Excited", 128, 64, ExcitedallArray_LEN, ExcitedallArray, 90},
            {"Fast", 128, 64, FastallArray_LEN, FastallArray, 90},
            {"Fierce", 128, 64, FierceallArray_LEN, FierceallArray, 90},
            {"Furious", 128, 64, FuriousallArray_LEN, FuriousallArray, 90},
            {"Giggle", 128, 64, GiggleallArray_LEN, GiggleallArray, 90},
            {"Glowing", 128, 64, GlowingallArray_LEN, GlowingallArray, 90},
            {"Growing", 128, 64, GrowingallArray_LEN, GrowingallArray, 90},
            {"intro", 128, 64, IntroallArray_LEN, IntroallArray, 80},
            {"Irritated", 128, 64, IrritatedallArray_LEN, IrritatedallArray, 90},
            {"Left", 128, 64, LeftallArray_LEN, LeftallArray, 90},
            {"Love", 128, 64, LoveallArray_LEN, LoveallArray, 90},
            {"Menacing", 128, 64, MenacingallArray_LEN, MenacingallArray, 90},
            {"Playful", 128, 64, PlayfulallArray_LEN, PlayfulallArray, 90},
            {"Rain", 128, 64, RainallArray_LEN, RainallArray, 90},
            {"Right", 128, 64, RightallArray_LEN, RightallArray, 90},
            {"Scared", 128, 64, ScaredallArray_LEN, ScaredallArray, 90},
            {"Serene", 128, 64, SereneallArray_LEN, SereneallArray, 90},
            {"Shrink", 128, 64, ShrinkallArray_LEN, ShrinkallArray, 90},
            {"Shy", 128, 64, ShyallArray_LEN, ShyallArray, 90},
            {"Sick", 128, 64, SickallArray_LEN, SickallArray, 90},
            {"Sleepy", 128, 64, SleepyallArray_LEN, SleepyallArray, 90},
            {"Smirk", 128, 64, SmirkallArray_LEN, SmirkallArray, 90},
            {"Smile", 128, 64, SmileallArray_LEN, SmileallArray, 90},
            {"Sneeze", 128, 64, SneezeallArray_LEN, SneezeallArray, 90},
            {"Sobbing", 128, 64, SobbingallArray_LEN, SobbingallArray, 90},
            {"Sparkle", 128, 64, SparkleallArray_LEN, SparkleallArray, 90},
            {"Splash", 128, 64, SplashallArray_LEN, SplashallArray, 90},
            {"Spraying", 128, 64, SprayingallArray_LEN, SprayingallArray, 90},
            {"Squint", 128, 64, SquintallArray_LEN, SquintallArray, 90},
            {"Surprised", 128, 64, SurprisedallArray_LEN, SurprisedallArray, 90},
            {"Swinging", 128, 64, SwingingallArray_LEN, SwingingallArray, 90},
            {"Yawn", 128, 64, YawnallArray_LEN, YawnallArray, 90},
        };
    }

    const AnimationClip &AnimationCatalog::get(AnimationId id)
    {
        const uint8_t index = static_cast<uint8_t>(id);
        if (index >= count())
        {
            return CLIPS[0];
        }

        return CLIPS[index];
    }

    const AnimationClip *AnimationCatalog::findByName(const char *name)
    {
        if (name == nullptr)
        {
            return nullptr;
        }

        for (uint8_t i = 0; i < count(); i++)
        {
            if (strcmp(CLIPS[i].name, name) == 0)
            {
                return &CLIPS[i];
            }
        }

        return nullptr;
    }

    uint8_t AnimationCatalog::count()
    {
        return sizeof(CLIPS) / sizeof(CLIPS[0]);
    }
}
