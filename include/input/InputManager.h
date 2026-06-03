#pragma once

#include <Arduino.h>

namespace robodesk
{
enum class InputEventType : uint8_t
{
    None,
    TapSequence,
    Hold,
    MenuHold
};

struct InputEvent
{
    InputEventType type = InputEventType::None;
    uint8_t tapCount = 0;
};

class InputManager
{
public:
    void begin();
    void update();
    InputEvent consumeEvent();

private:
    bool readPressed() const;
    void setEvent(InputEventType type, uint8_t tapCount = 0);

    InputEvent _pendingEvent;
    bool _lastRawPressed = false;
    bool _stablePressed = false;
    uint8_t _tapCount = 0;
    uint8_t _lastEmittedTapCount = 0;
    uint32_t _lastRawChangeAt = 0;
    uint32_t _pressStartedAt = 0;
    uint32_t _lastTapAt = 0;
};
}
