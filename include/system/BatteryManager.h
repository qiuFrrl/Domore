#pragma once

#include <Arduino.h>

namespace robodesk
{
struct BatteryStatus
{
    bool valid = false;
    uint16_t voltageMv = 0;
    uint8_t percent = 0;
    bool low = false;
};

class BatteryManager
{
public:
    void begin();
    void update(uint32_t nowMs);
    bool readNow();

    const BatteryStatus &status() const;

private:
    uint8_t estimatePercent(uint16_t voltageMv) const;

    BatteryStatus _status;
    uint32_t _lastReadAt = 0;
};
}
