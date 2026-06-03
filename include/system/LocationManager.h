#pragma once

#include <Arduino.h>

#include "system/LocationTypes.h"

namespace robodesk
{
class LocationManager
{
public:
    void begin();
    void update(uint32_t nowMs);
    void updateFromPhone(float latitude, float longitude);

    Coordinates current() const;
    bool usingPhoneLocation() const;

private:
    Coordinates _fallback;
    Coordinates _phone;
    uint32_t _lastPhoneFixAt = 0;
    bool _phoneFixValid = false;
};
}
