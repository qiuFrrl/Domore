#include "system/TimeManager.h"

#include <time.h>

#include "config/AppConfig.h"

namespace robodesk
{
void TimeManager::begin()
{
    configTime(
        TIME_GMT_OFFSET_SECONDS,
        TIME_DAYLIGHT_OFFSET_SECONDS,
        NTP_SERVER_PRIMARY,
        NTP_SERVER_SECONDARY);
}

void TimeManager::update(uint32_t nowMs)
{
    if (nowMs - _lastRefreshAt < TIME_REFRESH_MS)
    {
        return;
    }

    _lastRefreshAt = nowMs;
    syncNow();
}

bool TimeManager::syncNow()
{
    tm timeInfo;
    if (!getLocalTime(&timeInfo, 5))
    {
        _snapshot.valid = false;
        return false;
    }

    _snapshot.valid = true;
    _snapshot.hour = timeInfo.tm_hour;
    _snapshot.minute = timeInfo.tm_min;
    _snapshot.second = timeInfo.tm_sec;
    _snapshot.day = timeInfo.tm_mday;
    _snapshot.month = timeInfo.tm_mon + 1;
    _snapshot.year = timeInfo.tm_year + 1900;

    snprintf(_snapshot.timeText, sizeof(_snapshot.timeText), "%02u:%02u:%02u", _snapshot.hour, _snapshot.minute, _snapshot.second);
    snprintf(_snapshot.dateText, sizeof(_snapshot.dateText), "%04u/%02u/%02u", _snapshot.year, _snapshot.month, _snapshot.day);

    return true;
}

const TimeSnapshot &TimeManager::snapshot() const
{
    return _snapshot;
}
}
