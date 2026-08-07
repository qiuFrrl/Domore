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

    // Versi raw untuk networkTask: menerima flag koneksi langsung
    // tanpa perlu mengakses WifiManager dari thread berbeda.
    void updateStatus_raw(bool connected, bool wasConnected, uint32_t nowMs);

    static constexpr uint32_t HEARTBEAT_INTERVAL_MS = 30000; // 30 seconds

private:
    void setStatus(const char *status);
    void sendHeartbeat();
    bool _wasConnected = false;
    bool _statusSynced = false;
    uint32_t _lastHeartbeatMs = 0;
};
}
