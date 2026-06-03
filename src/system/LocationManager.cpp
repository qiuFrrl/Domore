#include "system/LocationManager.h"

#include "config/AppConfig.h"

namespace robodesk
{
void LocationManager::begin()
{
    _fallback.latitude = DEFAULT_LATITUDE;
    _fallback.longitude = DEFAULT_LONGITUDE;
    _fallback.valid = true;
}

void LocationManager::update(uint32_t nowMs)
{
    if (_phoneFixValid && nowMs - _lastPhoneFixAt > PHONE_LOCATION_MAX_AGE_MS)
    {
        _phoneFixValid = false;
    }
}

void LocationManager::updateFromPhone(float latitude, float longitude)
{
    _phone.latitude = latitude;
    _phone.longitude = longitude;
    _phone.valid = true;
    _lastPhoneFixAt = millis();
    _phoneFixValid = true;
}

Coordinates LocationManager::current() const
{
    if (_phoneFixValid && _phone.valid)
    {
        return _phone;
    }

    return _fallback;
}

bool LocationManager::usingPhoneLocation() const
{
    return _phoneFixValid && _phone.valid;
}
}
