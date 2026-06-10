#include "system/BatteryManager.h"

#include "config/AppConfig.h"

namespace robodesk
{
void BatteryManager::begin()
{
    pinMode(BATTERY_ADC_PIN, INPUT);
    analogReadResolution(12);
    analogSetPinAttenuation(BATTERY_ADC_PIN, ADC_11db);
    _status.voltage = readVoltage();
    _status.valid = _status.voltage > 0.1f;
    _status.low = _status.valid && _status.voltage <= BATTERY_LOW_VOLTAGE;
    _status.critical = _status.valid && _status.voltage <= BATTERY_CRITICAL_VOLTAGE;
}

void BatteryManager::update(uint32_t nowMs)
{
    if (_status.valid && nowMs - _lastReadAt < BATTERY_READ_INTERVAL_MS)
    {
        return;
    }

    _lastReadAt = nowMs;
    _status.voltage = readVoltage();
    _status.valid = _status.voltage > 0.1f;
    _status.low = _status.valid && _status.voltage <= BATTERY_LOW_VOLTAGE;
    _status.critical = _status.valid && _status.voltage <= BATTERY_CRITICAL_VOLTAGE;
}

const BatteryStatus &BatteryManager::status() const
{
    return _status;
}

float BatteryManager::readVoltage() const
{
    uint32_t millivolts = 0;
    for (uint8_t i = 0; i < BATTERY_SAMPLE_COUNT; i++)
    {
        millivolts += analogReadMilliVolts(BATTERY_ADC_PIN);
        delay(2);
    }

    const float adcVolts = (millivolts / static_cast<float>(BATTERY_SAMPLE_COUNT)) / 1000.0f;
    const float dividerRatio = (BATTERY_DIVIDER_TOP_OHMS + BATTERY_DIVIDER_BOTTOM_OHMS) / BATTERY_DIVIDER_BOTTOM_OHMS;
    return adcVolts * dividerRatio;
}
}
