#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <vector>

#include "system/WifiTypes.h"

namespace robodesk
{
enum class WifiState : uint8_t
{
    Idle,
    Connecting,
    Connected,
    WaitingRetry,
    Disabled
};

class WifiManager
{
public:
    void begin(const WifiCredential *credentials, uint8_t credentialCount);
    void update(uint32_t nowMs);

    void addCredential(const char *ssid, const char *password);
    void clearDynamicCredentials();
    void forceConnect();

    bool isConnected() const;
    bool hasCredentials() const;
    WifiState state() const;
    const char *activeSsid() const;
    IPAddress ipAddress() const;

private:
    void startNextConnection(uint32_t nowMs);
    bool credentialIsUsable(size_t index) const;
    void loadPreferences();
    void savePreferences();

    std::vector<WifiCredential> _credentials;
    size_t _currentIndex = 0;
    uint32_t _connectStartedAt = 0;
    uint32_t _nextRetryAt = 0;
    WifiState _state = WifiState::Idle;
};
}

