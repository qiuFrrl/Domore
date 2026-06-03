#pragma once

#include <Arduino.h>
#include <U8g2lib.h>

#include "display/AnimationTypes.h"

namespace robodesk
{
class AnimationPlayer
{
public:
    void play(const AnimationClip &clip, uint16_t loopCount = 0, bool restartIfSame = false);
    void stop();
    void update(uint32_t nowMs);
    void draw(U8G2 &display, int16_t x = 0, int16_t y = 0) const;

    bool isRunning() const;
    bool isFinished() const;
    const AnimationClip *currentClip() const;

private:
    const AnimationClip *_clip = nullptr;
    uint16_t _frameIndex = 0;
    uint16_t _completedLoops = 0;
    uint16_t _requestedLoops = 0;
    uint32_t _lastFrameAt = 0;
    bool _running = false;
    bool _finished = false;
};
}
