#pragma once

#include <Arduino.h>

#include "display/AnimationTypes.h"

namespace robodesk
{
    enum class AnimationId : uint8_t
    {
        Blank,         // Special case for stopping and clearing the display.
        Buzzing,       // default
        Devil,         // 18-8
        Distracted,    // default
        Down,          // default
        Drowsy,        // tap count 1-2
        Encouragement, // inactive hold wake
        Enraged,       // tap count 5+
        Excited,       // inactive tap wake
        Fast,          // default
        Fierce,        // tap count 9-11
        Furious,       // tap count 8-10
        Giggle,        // default
        Glowing,       // 18-8
        Growing,       // default
        Intro,         // boot
        Irritated,     // tap count 6-8
        Left,          // default
        Love,          // birthday final
        Menacing,      // 18-8
        Playful,       // default
        Rain,          // default,
        Right,         // default
        Scared,        // 18-8
        Serene,        // default
        Shrink,        // default
        Shy,           // hold
        Sick,          // default
        Sleepy,        // inactive
        Smirk,         // default
        Smile,         // default
        Sneeze,        // default
        Sobbing,       // 18-8
        Sparkle,       // default
        Splash,        // default
        Spraying,      // default
        Squint,        // tap count 3-5
        Surprised,     // default
        Swinging,      // tap count 12+
        Yawn,          // low battery
        Count
    };

    namespace AnimationCatalog
    {
        const AnimationClip &get(AnimationId id);
        const AnimationClip *findByName(const char *name);
        uint8_t count();
    }
}
