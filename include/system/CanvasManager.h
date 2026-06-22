#pragma once

#include <Arduino.h>

namespace robodesk
{
struct CanvasData
{
    uint8_t buffer[1024]; // 128x64 1-bit XBM = 1024 bytes
    bool hasNew = false;  // ada gambar baru belum dilihat
    bool hasEver = false; // pernah terima gambar sejak boot
};

class CanvasManager
{
public:
    void update(uint32_t nowMs, bool wifiConnected, const char *pairCode);

    bool hasNew() const;
    bool hasEver() const;
    void markSeen();
    const uint8_t *buffer() const;

private:
    bool pollUpdatedAt(const char *pairCode);
    void fetchPixels(const char *pairCode);
    void decodeHex(const char *hexStr, uint16_t len);

    CanvasData _data;
    uint32_t _lastPollMs = 0;
    uint64_t _lastUpdatedAt = 0;

    static constexpr uint32_t POLL_INTERVAL_MS = 8000;
};
}
