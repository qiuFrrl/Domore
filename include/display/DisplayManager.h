#pragma once

#include <Arduino.h>
#include <U8g2lib.h>

#include "display/AnimationPlayer.h"
#include "display/BirthdayScene.h"
#include "menu/MenuManager.h"
#include "system/BatteryManager.h"
#include "system/TimeManager.h"
#include "system/WeatherManager.h"

namespace robodesk
{
enum class ScreenId : uint8_t
{
    Boot,
    Domore,
    Menu,
    Time,
    Weather,
    Birthday,
    Wifi,
    Battery,
    Canvas
};

enum class WifiScreenState : uint8_t
{
    Loading,
    Connected,
    Failed
};

class DisplayManager
{
public:
    DisplayManager();

    void begin();
    bool shouldRender(uint32_t nowMs);
    void setInvert(bool invert);
    void render(
        ScreenId screen,
        const AnimationPlayer &animation,
        const MenuManager &menu,
        const TimeSnapshot &time,
        const WeatherData &weather,
        const BatteryStatus &battery,
        const BirthdayScene &birthday,
        WifiScreenState wifiState,
        uint32_t wifiElapsedMs,
        const uint8_t* canvasBuffer,
        bool canvasHasEver,
        bool canvasHasNew,
        uint32_t nowMs);

    U8G2 &raw();

private:
    void drawDomore(const AnimationPlayer &animation, const TimeSnapshot &time, const WeatherData &weather, const BatteryStatus &battery);
    void drawBoot(const AnimationPlayer &animation);
    void drawMenu(const MenuManager &menu);
    void drawTime(const TimeSnapshot &time);
    void drawWeather(const WeatherData &weather);
    void drawWeatherIcon(const WeatherData &weather, int16_t x, int16_t y);
    void drawBirthday(const BirthdayScene &birthday, uint32_t nowMs);
    void drawWifi(WifiScreenState state, uint32_t elapsedMs);
    void drawBattery(const BatteryStatus &battery);
    void drawStatusBar(const TimeSnapshot &time, const WeatherData &weather);
    void drawBatteryWarning(const BatteryStatus &battery);
    void drawCentered(const char *text, int16_t y);
    void drawCanvas(const uint8_t* buffer, bool hasEver);
    void drawCanvasNotification();

    U8G2_SH1106_128X64_NONAME_F_HW_I2C _oled;
    uint32_t _lastRenderAt = 0;
};
}
