#pragma once

#include <Arduino.h>

namespace robodesk
{
struct TimeSnapshot
{
    bool valid = false;
    char timeText[9] = "--:--:--";
    char dateText[11] = "----/--/--";
    uint8_t hour = 0;
    uint8_t minute = 0;
    uint8_t second = 0;
    uint8_t day = 0;
    uint8_t month = 0;
    uint16_t year = 0;
};

class TimeManager
{
public:
    void begin();
    void update(uint32_t nowMs);
    bool syncNow();

    const TimeSnapshot &snapshot() const;

private:
    TimeSnapshot _snapshot;
    uint32_t _lastRefreshAt = 0;
};
}
