#include "display/AnimationPlayer.h"

namespace robodesk
{
void AnimationPlayer::play(const AnimationClip &clip, uint16_t loopCount, bool restartIfSame)
{
    if (!restartIfSame && _clip == &clip && _running)
    {
        return;
    }

    _clip = &clip;
    _frameIndex = 0;
    _completedLoops = 0;
    _requestedLoops = loopCount;
    _lastFrameAt = millis();
    _running = clip.frameCount > 0;
    _finished = !_running;
}

void AnimationPlayer::stop()
{
    _running = false;
    _finished = true;
}

void AnimationPlayer::update(uint32_t nowMs)
{
    if (!_running || _clip == nullptr || _clip->frameCount == 0)
    {
        return;
    }

    const uint16_t delayMs = _clip->frameDelayMs == 0 ? 80 : _clip->frameDelayMs;
    if (nowMs - _lastFrameAt < delayMs)
    {
        return;
    }

    _lastFrameAt = nowMs;
    _frameIndex++;

    if (_frameIndex < _clip->frameCount)
    {
        return;
    }

    _frameIndex = 0;
    if (_requestedLoops == 0)
    {
        return;
    }

    _completedLoops++;
    if (_completedLoops >= _requestedLoops)
    {
        _frameIndex = _clip->frameCount - 1;
        _running = false;
        _finished = true;
    }
}

void AnimationPlayer::draw(U8G2 &display, int16_t x, int16_t y) const
{
    if (_clip == nullptr || _clip->frameCount == 0 || _clip->frames == nullptr)
    {
        return;
    }

    display.drawXBMP(x, y, _clip->width, _clip->height, _clip->frames[_frameIndex]);
}

bool AnimationPlayer::isRunning() const
{
    return _running;
}

bool AnimationPlayer::isFinished() const
{
    return _finished;
}

const AnimationClip *AnimationPlayer::currentClip() const
{
    return _clip;
}
}
