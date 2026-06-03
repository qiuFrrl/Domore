#pragma once

#include <Arduino.h>

namespace robodesk
{
    struct AnimationClip
    {
        const char *name;
        uint8_t width;
        uint8_t height;
        uint16_t frameCount;
        const uint8_t *const *frames;
        uint16_t frameDelayMs;
    };
}
