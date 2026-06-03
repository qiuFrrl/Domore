#include "system/BatteryManager.h"

#include "config/AppConfig.h"

namespace robodesk
{
void BatteryManager::begin()
{
    analogReadResolution(12);
    analogSetPinAttenuation(BATTERY_PIN, ADC_11db);
    readNow();
}

void BatteryManager::update(uint32_t nowMs)
{
    if (_lastReadAt != 0 && nowMs - _lastReadAt < BATTERY_READ_INTERVAL_MS)
    {
        return;
    }

    readNow();
}

bool BatteryManager::readNow()
{
    constexpr uint8_t sampleCount = 8;
    uint32_t adcMvTotal = 0;

    for (uint8_t i = 0; i < sampleCount; i++)
    {
        adcMvTotal += analogReadMilliVolts(BATTERY_PIN);
        delayMicroseconds(200);
    }

    const float adcMv = static_cast<float>(adcMvTotal) / sampleCount;
    const uint16_t batteryMv = static_cast<uint16_t>((adcMv * BATTERY_DIVIDER_RATIO) + 0.5f);

    _lastReadAt = millis();
    _status.valid = batteryMv >= BATTERY_VALID_MIN_MV;

    if (!_status.valid)
    {
        _status.voltageMv = 0;
        _status.percent = 0;
        _status.low = false;
        return false;
    }

    _status.voltageMv = batteryMv;
    _status.percent = estimatePercent(batteryMv);

    if (_status.low)
    {
        _status.low = batteryMv < BATTERY_RECOVER_MV;
    }
    else
    {
        _status.low = batteryMv <= BATTERY_LOW_MV;
    }

    return true;
}

const BatteryStatus &BatteryManager::status() const
{
    return _status;
}

uint8_t BatteryManager::estimatePercent(uint16_t voltageMv) const
{
    if (voltageMv >= 4200)
    {
        return 100;
    }
    if (voltageMv >= 4100)
    {
        return 90;
    }
    if (voltageMv >= 4000)
    {
        return 80;
    }
    if (voltageMv >= 3900)
    {
        return 65;
    }
    if (voltageMv >= 3800)
    {
        return 50;
    }
    if (voltageMv >= 3700)
    {
        return 35;
    }
    if (voltageMv >= 3600)
    {
        return 20;
    }
    if (voltageMv >= 3500)
    {
        return 10;
    }
    if (voltageMv >= 3400)
    {
        return 5;
    }

    return 0;
}
}
