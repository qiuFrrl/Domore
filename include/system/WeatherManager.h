#pragma once

#include <Arduino.h>

#include "system/LocationTypes.h"

namespace robodesk
{
struct WeatherData
{
    bool valid = false;
    float temperatureC = 0.0f;
    uint8_t humidity = 0;
    uint16_t weatherCode = 0;
    char summary[20] = "Waiting";
    uint32_t updatedAtMs = 0;
};

class WeatherManager
{
public:
    void begin();
    void update(uint32_t nowMs, bool wifiConnected, Coordinates coordinates);
    bool refresh(Coordinates coordinates);

    const WeatherData &data() const;

private:
    const char *summaryForCode(uint16_t code) const;

    WeatherData _data;
    uint32_t _lastAttemptAt = 0;
};
}
