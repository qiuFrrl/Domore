#pragma once

#include <Arduino.h>

#include "system/WifiTypes.h"

namespace robodesk
{
    constexpr uint8_t DISPLAY_WIDTH = 128;
    constexpr uint8_t DISPLAY_HEIGHT = 64;
    constexpr uint16_t DISPLAY_FRAME_INTERVAL_MS = 33;

#ifndef ROBODESK_OLED_SDA_PIN
#define ROBODESK_OLED_SDA_PIN 8
#endif

#ifndef ROBODESK_OLED_SCL_PIN
#define ROBODESK_OLED_SCL_PIN 9
#endif

    constexpr int OLED_SDA_PIN = ROBODESK_OLED_SDA_PIN;
    constexpr int OLED_SCL_PIN = ROBODESK_OLED_SCL_PIN;

#ifndef ROBODESK_BUTTON_PIN
#define ROBODESK_BUTTON_PIN 4
#endif

    constexpr int BUTTON_PIN = ROBODESK_BUTTON_PIN;
    constexpr bool BUTTON_ACTIVE_LOW = false;
    constexpr bool BUTTON_INTERNAL_PULLUP = false;
    constexpr uint16_t BUTTON_DEBOUNCE_MS = 35;
    constexpr uint16_t BUTTON_TAP_GAP_MS = 650;
    constexpr uint16_t BUTTON_TAP_RESET_MS = 8000;
    constexpr uint16_t BUTTON_HOLD_MS = 800;
    constexpr uint16_t BUTTON_MENU_HOLD_MS = 5000;

#ifndef ROBODESK_BATTERY_PIN
#define ROBODESK_BATTERY_PIN 3
#endif

    constexpr int BATTERY_PIN = ROBODESK_BATTERY_PIN;
    constexpr float BATTERY_DIVIDER_RATIO = 2.0f;
    constexpr uint16_t BATTERY_VALID_MIN_MV = 2500;
    constexpr uint16_t BATTERY_LOW_MV = 3500;
    constexpr uint16_t BATTERY_RECOVER_MV = 3700;
    constexpr uint32_t BATTERY_READ_INTERVAL_MS = 10000;

    static const WifiCredential WIFI_CREDENTIALS[] = {
        {"bitha", "apayalupa"},
        {"Lich 5G", "rafles5o"},
    };
    constexpr uint8_t WIFI_CREDENTIAL_COUNT = sizeof(WIFI_CREDENTIALS) / sizeof(WIFI_CREDENTIALS[0]);
    constexpr uint32_t WIFI_CONNECT_TIMEOUT_MS = 12000;
    constexpr uint32_t WIFI_RETRY_INTERVAL_MS = 15000;

    constexpr long TIME_GMT_OFFSET_SECONDS = 7L * 60L * 60L;
    constexpr int TIME_DAYLIGHT_OFFSET_SECONDS = 0;
    constexpr const char *NTP_SERVER_PRIMARY = "pool.ntp.org";
    constexpr const char *NTP_SERVER_SECONDARY = "time.nist.gov";
    constexpr uint32_t TIME_REFRESH_MS = 1000;

    // Static fallback location. Phone BLE GPS can override this at runtime.
    constexpr float DEFAULT_LATITUDE = 13.7563f;
    constexpr float DEFAULT_LONGITUDE = 100.5018f;
    constexpr uint32_t PHONE_LOCATION_MAX_AGE_MS = 10UL * 60UL * 1000UL;

    constexpr uint32_t WEATHER_REFRESH_MS = 5UL * 60UL * 1000UL;
    constexpr uint32_t WEATHER_RETRY_MS = 5UL * 60UL * 1000UL;

    constexpr uint16_t HOME_RANDOM_DELAY_MIN_MS = 1000;
    constexpr uint16_t HOME_RANDOM_DELAY_MAX_MS = 5000;
    constexpr uint32_t BOOT_INTRO_LAST_FRAME_HOLD_MS = 2000;
    constexpr uint32_t BUTTON_INACTIVITY_SLEEP_MS = 2UL * 60UL * 1000UL;
    constexpr uint32_t LOW_BATTERY_ANIMATION_REMINDER_MS = 5UL * 60UL * 1000UL;
    constexpr uint8_t BIRTHDAY_DAY = 3;
    constexpr uint8_t BIRTHDAY_MONTH = 6;
    constexpr uint32_t BIRTHDAY_SCENE_MS = 12000;
}
