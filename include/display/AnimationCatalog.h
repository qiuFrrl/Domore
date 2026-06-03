#pragma once

#include <Arduino.h>

#include "display/AnimationTypes.h"

namespace robodesk
{
enum class AnimationId : uint8_t
{
    Blank,
    Buzzing,
    Devil,
    Distracted,
    Down,
    Drowsy,
    Encouragement,
    Enraged,
    Excited,
    Fast,
    Fierce,
    Furious,
    Giggle,
    Glowing,
    Growing,
    Intro,
    Irritated,
    Left,
    Love,
    Menacing,
    Playful,
    Rain,
    Right,
    Scared,
    Serene,
    Shrink,
    Shy,
    Sick,
    Sleepy,
    Smirk,
    Smile,
    Sneeze,
    Sobbing,
    Sparkle,
    Splash,
    Spraying,
    Squint,
    Surprised,
    Swinging,
    Yawn,
    Count
};

namespace AnimationCatalog
{
const AnimationClip &get(AnimationId id);
const AnimationClip *findByName(const char *name);
uint8_t count();
}
}
