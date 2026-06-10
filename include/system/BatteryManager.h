#pragma once

#include <Arduino.h>

namespace robodesk
{
struct BatteryStatus
{
    bool valid = false;
    float voltage = 0.0f;
    bool low = false;
    bool critical = false;
};

class BatteryManager
{
public:
    void begin();
    void update(uint32_t nowMs);

    const BatteryStatus &status() const;

private:
    float readVoltage() const;

    BatteryStatus _status;
    uint32_t _lastReadAt = 0;
};
}
