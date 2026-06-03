#include "input/InputManager.h"

#include "config/AppConfig.h"

namespace robodesk
{
void InputManager::begin()
{
    pinMode(BUTTON_PIN, BUTTON_INTERNAL_PULLUP ? INPUT_PULLUP : INPUT);

    const uint32_t nowMs = millis();
    _lastRawPressed = readPressed();
    _stablePressed = _lastRawPressed;
    _lastRawChangeAt = nowMs;
    _pressStartedAt = _stablePressed ? nowMs : 0;
}

void InputManager::update()
{
    const uint32_t nowMs = millis();
    const bool rawPressed = readPressed();

    if (rawPressed != _lastRawPressed)
    {
        _lastRawPressed = rawPressed;
        _lastRawChangeAt = nowMs;
    }

    if (nowMs - _lastRawChangeAt >= BUTTON_DEBOUNCE_MS && rawPressed != _stablePressed)
    {
        _stablePressed = rawPressed;

        if (_stablePressed)
        {
            _pressStartedAt = nowMs;
        }
        else
        {
            const uint32_t heldMs = nowMs - _pressStartedAt;
            if (heldMs >= BUTTON_MENU_HOLD_MS)
            {
                _tapCount = 0;
                _lastEmittedTapCount = 0;
                setEvent(InputEventType::MenuHold);
            }
            else if (heldMs >= BUTTON_HOLD_MS)
            {
                _tapCount = 0;
                _lastEmittedTapCount = 0;
                setEvent(InputEventType::Hold);
            }
            else
            {
                if (_tapCount < 250)
                {
                    _tapCount++;
                }
                _lastTapAt = nowMs;
            }
        }
    }

    if (!_stablePressed &&
        _tapCount > 0 &&
        _lastEmittedTapCount != _tapCount &&
        nowMs - _lastTapAt >= BUTTON_TAP_GAP_MS)
    {
        setEvent(InputEventType::TapSequence, _tapCount);
        _lastEmittedTapCount = _tapCount;
    }

    if (!_stablePressed && _tapCount > 0 && nowMs - _lastTapAt >= BUTTON_TAP_RESET_MS)
    {
        _tapCount = 0;
        _lastEmittedTapCount = 0;
    }
}

InputEvent InputManager::consumeEvent()
{
    const InputEvent event = _pendingEvent;
    _pendingEvent = {};
    return event;
}

bool InputManager::readPressed() const
{
    const int level = digitalRead(BUTTON_PIN);
    return BUTTON_ACTIVE_LOW ? level == LOW : level == HIGH;
}

void InputManager::setEvent(InputEventType type, uint8_t tapCount)
{
    if (_pendingEvent.type != InputEventType::None)
    {
        return;
    }

    _pendingEvent.type = type;
    _pendingEvent.tapCount = tapCount;
}
}
