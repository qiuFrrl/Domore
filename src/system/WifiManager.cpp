#include "system/WifiManager.h"

#include "config/AppConfig.h"

namespace robodesk
{
void WifiManager::begin(const WifiCredential *credentials, uint8_t credentialCount)
{
    _credentials = credentials;
    _credentialCount = credentialCount;
    _currentIndex = 0;
    _connectStartedAt = 0;
    _nextRetryAt = 0;
    _state = hasCredentials() ? WifiState::Idle : WifiState::Disabled;

    WiFi.persistent(false);
    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(true);
}

void WifiManager::update(uint32_t nowMs)
{
    if (_state == WifiState::Disabled)
    {
        return;
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        _state = WifiState::Connected;
        return;
    }

    if (_state == WifiState::Connected)
    {
        _state = WifiState::WaitingRetry;
        _nextRetryAt = nowMs + WIFI_RETRY_INTERVAL_MS;
        return;
    }

    if (_state == WifiState::Connecting)
    {
        if (nowMs - _connectStartedAt >= WIFI_CONNECT_TIMEOUT_MS)
        {
            _currentIndex++;
            startNextConnection(nowMs);
        }
        return;
    }

    if (_state == WifiState::WaitingRetry && nowMs < _nextRetryAt)
    {
        return;
    }

    startNextConnection(nowMs);
}

bool WifiManager::isConnected() const
{
    return WiFi.status() == WL_CONNECTED;
}

bool WifiManager::hasCredentials() const
{
    if (_credentials == nullptr || _credentialCount == 0)
    {
        return false;
    }

    for (uint8_t i = 0; i < _credentialCount; i++)
    {
        if (credentialIsUsable(i))
        {
            return true;
        }
    }

    return false;
}

WifiState WifiManager::state() const
{
    return _state;
}

const char *WifiManager::activeSsid() const
{
    if (_credentials == nullptr || _currentIndex >= _credentialCount)
    {
        return "";
    }

    return _credentials[_currentIndex].ssid;
}

IPAddress WifiManager::ipAddress() const
{
    return WiFi.localIP();
}

void WifiManager::startNextConnection(uint32_t nowMs)
{
    if (!hasCredentials())
    {
        _state = WifiState::Disabled;
        return;
    }

    for (uint8_t attempt = 0; attempt < _credentialCount; attempt++)
    {
        const uint8_t index = (_currentIndex + attempt) % _credentialCount;
        if (!credentialIsUsable(index))
        {
            continue;
        }

        _currentIndex = index;
        WiFi.disconnect(false, false);
        WiFi.begin(_credentials[_currentIndex].ssid, _credentials[_currentIndex].password);
        _connectStartedAt = nowMs;
        _state = WifiState::Connecting;
        return;
    }

    _state = WifiState::Disabled;
}

bool WifiManager::credentialIsUsable(uint8_t index) const
{
    if (_credentials == nullptr || index >= _credentialCount)
    {
        return false;
    }

    const char *ssid = _credentials[index].ssid;
    return ssid != nullptr && ssid[0] != '\0';
}
}
