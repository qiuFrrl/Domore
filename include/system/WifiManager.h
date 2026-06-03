#pragma once

#include <Arduino.h>
#include <WiFi.h>

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

    bool isConnected() const;
    bool hasCredentials() const;
    WifiState state() const;
    const char *activeSsid() const;
    IPAddress ipAddress() const;

private:
    void startNextConnection(uint32_t nowMs);
    bool credentialIsUsable(uint8_t index) const;

    const WifiCredential *_credentials = nullptr;
    uint8_t _credentialCount = 0;
    uint8_t _currentIndex = 0;
    uint32_t _connectStartedAt = 0;
    uint32_t _nextRetryAt = 0;
    WifiState _state = WifiState::Idle;
};
}
