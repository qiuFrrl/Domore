#include "system/CanvasManager.h"

#include <HTTPClient.h>
#include <WiFiClientSecure.h>

#include "database.h"

namespace robodesk
{
void CanvasManager::update(uint32_t nowMs, bool wifiConnected, const char *pairCode)
{
    if (!wifiConnected)
    {
        return;
    }

    if (nowMs - _lastPollMs < POLL_INTERVAL_MS)
    {
        return;
    }

    _lastPollMs = nowMs;

    if (pollUpdatedAt(pairCode))
    {
        fetchPixels(pairCode);
    }
}

bool CanvasManager::hasNew() const
{
    return _data.hasNew;
}

bool CanvasManager::hasEver() const
{
    return _data.hasEver;
}

void CanvasManager::markSeen()
{
    _data.hasNew = false;
}

const uint8_t *CanvasManager::buffer() const
{
    return _data.buffer;
}

bool CanvasManager::pollUpdatedAt(const char *pairCode)
{
    WiFiClientSecure client;
    client.setInsecure();

    HTTPClient http;
    http.setTimeout(4000);
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);

    String url = String(DATABASE_URL) + "/robot/canvas/" + pairCode + "/updatedAt.json";
    if (!http.begin(client, url))
    {
        Serial.println("CanvasManager: updatedAt HTTP begin failed");
        return false;
    }

    const int status = http.GET();
    if (status != HTTP_CODE_OK)
    {
        http.end();
        Serial.printf("CanvasManager: updatedAt HTTP %d\n", status);
        return false;
    }

    const String payload = http.getString();
    http.end();

    // payload is a raw number like 1781197449576 or "null"
    if (payload == "null" || payload.length() == 0)
    {
        return false;
    }

    const uint64_t remoteUpdatedAt = strtoull(payload.c_str(), nullptr, 10);
    if (remoteUpdatedAt == 0 || remoteUpdatedAt == _lastUpdatedAt)
    {
        return false;
    }

    _lastUpdatedAt = remoteUpdatedAt;
    Serial.printf("CanvasManager: New canvas detected, updatedAt=%llu\n", remoteUpdatedAt);
    return true;
}

void CanvasManager::fetchPixels(const char *pairCode)
{
    WiFiClientSecure client;
    client.setInsecure();

    HTTPClient http;
    http.setTimeout(6000);
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);

    String url = String(DATABASE_URL) + "/robot/canvas/" + pairCode + "/pixels.json";
    if (!http.begin(client, url))
    {
        Serial.println("CanvasManager: pixels HTTP begin failed");
        return;
    }

    const int status = http.GET();
    if (status != HTTP_CODE_OK)
    {
        http.end();
        Serial.printf("CanvasManager: pixels HTTP %d\n", status);
        return;
    }

    const String payload = http.getString();
    http.end();

    // payload is a quoted JSON string: "ff00a1..."
    // Strip surrounding quotes
    if (payload.length() < 4)
    {
        Serial.println("CanvasManager: pixels payload too short");
        return;
    }

    // Find the hex content between quotes
    const int startQuote = payload.indexOf('"');
    const int endQuote = payload.lastIndexOf('"');
    if (startQuote < 0 || endQuote <= startQuote)
    {
        Serial.println("CanvasManager: pixels no quotes found");
        return;
    }

    const char *hexStart = payload.c_str() + startQuote + 1;
    const uint16_t hexLen = endQuote - startQuote - 1;

    if (hexLen < 2048)
    {
        Serial.printf("CanvasManager: pixels hex too short (%u)\n", hexLen);
        return;
    }

    decodeHex(hexStart, 2048);

    _data.hasNew = true;
    _data.hasEver = true;
    Serial.println("CanvasManager: Canvas image decoded OK");
}

void CanvasManager::decodeHex(const char *hexStr, uint16_t len)
{
    const uint16_t byteCount = len / 2;
    const uint16_t limit = (byteCount > 1024) ? 1024 : byteCount;

    for (uint16_t i = 0; i < limit; i++)
    {
        char hexByte[3];
        hexByte[0] = hexStr[i * 2];
        hexByte[1] = hexStr[i * 2 + 1];
        hexByte[2] = '\0';
        _data.buffer[i] = (uint8_t)strtoul(hexByte, nullptr, 16);
    }
}
}
