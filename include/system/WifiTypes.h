#pragma once

#include <Arduino.h>
#include <string.h>

namespace robodesk
{
constexpr uint8_t WIFI_SSID_MAX_LEN = 33;     // 32 chars + null (IEEE 802.11 max)
constexpr uint8_t WIFI_PASSWORD_MAX_LEN = 64;  // 63 chars + null (WPA max)

struct WifiCredential
{
    char ssid[WIFI_SSID_MAX_LEN];
    char password[WIFI_PASSWORD_MAX_LEN];
};
}
