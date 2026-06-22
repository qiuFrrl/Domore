#pragma once

#include <Arduino.h>
#include "system/WifiManager.h"

namespace robodesk
{
class FirebaseManager
{
public:
    void fetchWifi(WifiManager &wifiManager);
    void updateStatus(WifiManager &wifiManager, uint32_t nowMs);

    static constexpr uint32_t HEARTBEAT_INTERVAL_MS = 60000; // 1 minute

private:
    void setStatus(const char *status);
    void sendHeartbeat();
    bool _wasConnected = false;
    bool _statusSynced = false;
    uint32_t _lastHeartbeatMs = 0;
};
}
