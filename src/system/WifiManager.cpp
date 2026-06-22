#include "system/WifiManager.h"

#include "config/AppConfig.h"
#include <Preferences.h>
#include <string.h>

namespace robodesk
{
void WifiManager::begin(const WifiCredential *credentials, uint8_t credentialCount)
{
    _credentials.clear();
    for (uint8_t i = 0; i < credentialCount; i++)
    {
        WifiCredential cred;
        strncpy(cred.ssid, credentials[i].ssid, WIFI_SSID_MAX_LEN);
        cred.ssid[WIFI_SSID_MAX_LEN - 1] = '\0';
        strncpy(cred.password, credentials[i].password, WIFI_PASSWORD_MAX_LEN);
        cred.password[WIFI_PASSWORD_MAX_LEN - 1] = '\0';
        _credentials.push_back(cred);
    }

    loadPreferences();

    _currentIndex = 0;
    _connectStartedAt = 0;
    _nextRetryAt = 0;
    _state = hasCredentials() ? WifiState::Idle : WifiState::Disabled;

    WiFi.persistent(false);
    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(true);
}

void WifiManager::loadPreferences()
{
    Preferences prefs;
    prefs.begin("wifi_db", true);
    uint32_t count = prefs.getUInt("count", 0);
    for (uint32_t i = 0; i < count; i++)
    {
        char ssidKey[8];
        char passKey[8];
        snprintf(ssidKey, sizeof(ssidKey), "s_%lu", (unsigned long)i);
        snprintf(passKey, sizeof(passKey), "p_%lu", (unsigned long)i);

        WifiCredential cred;
        memset(&cred, 0, sizeof(cred));

        String ssidStr = prefs.getString(ssidKey, "");
        String passStr = prefs.getString(passKey, "");

        if (ssidStr.length() > 0)
        {
            strncpy(cred.ssid, ssidStr.c_str(), WIFI_SSID_MAX_LEN);
            cred.ssid[WIFI_SSID_MAX_LEN - 1] = '\0';
            strncpy(cred.password, passStr.c_str(), WIFI_PASSWORD_MAX_LEN);
            cred.password[WIFI_PASSWORD_MAX_LEN - 1] = '\0';
            _credentials.push_back(cred);
        }
    }
    prefs.end();
}

void WifiManager::savePreferences()
{
    Preferences prefs;
    prefs.begin("wifi_db", false);
    prefs.clear();

    uint32_t dynamicCount = 0;
    if (_credentials.size() > WIFI_CREDENTIAL_COUNT)
    {
        dynamicCount = _credentials.size() - WIFI_CREDENTIAL_COUNT;
    }

    prefs.putUInt("count", dynamicCount);
    for (uint32_t i = 0; i < dynamicCount; i++)
    {
        char ssidKey[8];
        char passKey[8];
        snprintf(ssidKey, sizeof(ssidKey), "s_%lu", (unsigned long)i);
        snprintf(passKey, sizeof(passKey), "p_%lu", (unsigned long)i);
        const auto &cred = _credentials[WIFI_CREDENTIAL_COUNT + i];
        prefs.putString(ssidKey, cred.ssid);
        prefs.putString(passKey, cred.password);
    }
    prefs.end();
}

void WifiManager::addCredential(const char *ssid, const char *password)
{
    if (ssid == nullptr || ssid[0] == '\0') return;

    for (const auto &cred : _credentials)
    {
        if (strcmp(cred.ssid, ssid) == 0 && strcmp(cred.password, password) == 0)
        {
            return;
        }
    }

    WifiCredential cred;
    strncpy(cred.ssid, ssid, WIFI_SSID_MAX_LEN);
    cred.ssid[WIFI_SSID_MAX_LEN - 1] = '\0';
    strncpy(cred.password, password != nullptr ? password : "", WIFI_PASSWORD_MAX_LEN);
    cred.password[WIFI_PASSWORD_MAX_LEN - 1] = '\0';
    _credentials.push_back(cred);
    savePreferences();
}

void WifiManager::clearDynamicCredentials()
{
    if (_credentials.size() > WIFI_CREDENTIAL_COUNT)
    {
        _credentials.erase(_credentials.begin() + WIFI_CREDENTIAL_COUNT, _credentials.end());
        if (_currentIndex >= _credentials.size())
        {
            _currentIndex = 0;
        }
        savePreferences();
    }
}

void WifiManager::forceConnect()
{
    if (!hasCredentials()) return;

    _state = WifiState::Idle;
    _currentIndex = (_currentIndex + 1) % _credentials.size();
    _nextRetryAt = 0;
}

void WifiManager::update(uint32_t nowMs)
{
    if (_state == WifiState::Disabled) return;

    if (WiFi.status() == WL_CONNECTED)
    {
        _state = WifiState::Connected;
        return;
    }

    if (_state == WifiState::Connected)
    {
        _state = WifiState::WaitingRetry;
        _nextRetryAt = nowMs + 10000;
        return;
    }

    if (_state == WifiState::Connecting)
    {
        if (nowMs - _connectStartedAt >= 10000)
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
    if (_credentials.empty()) return false;
    for (size_t i = 0; i < _credentials.size(); i++)
    {
        if (credentialIsUsable(i)) return true;
    }
    return false;
}

WifiState WifiManager::state() const
{
    return _state;
}

const char *WifiManager::activeSsid() const
{
    if (_currentIndex >= _credentials.size()) return "";
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

    for (size_t attempt = 0; attempt < _credentials.size(); attempt++)
    {
        const size_t index = (_currentIndex + attempt) % _credentials.size();
        if (!credentialIsUsable(index)) continue;

        _currentIndex = index;
        WiFi.disconnect(false, false);
        WiFi.begin(_credentials[_currentIndex].ssid, _credentials[_currentIndex].password);
        _connectStartedAt = nowMs;
        _state = WifiState::Connecting;

        Serial.printf("WifiManager: Connecting to %s\n", _credentials[_currentIndex].ssid);
        return;
    }

    _state = WifiState::Disabled;
}

bool WifiManager::credentialIsUsable(size_t index) const
{
    if (index >= _credentials.size()) return false;
    return _credentials[index].ssid[0] != '\0';
}
}
