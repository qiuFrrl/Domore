#include "display/DisplayManager.h"

#include <stdio.h>
#include <Wire.h>

#include "config/AppConfig.h"

namespace robodesk
{
DisplayManager::DisplayManager()
    : _oled(U8G2_R0, U8X8_PIN_NONE)
{
}

void DisplayManager::begin()
{
    Wire.begin(OLED_SDA_PIN, OLED_SCL_PIN);
    _oled.begin();
    _oled.setFont(u8g2_font_5x8_tf);
    _oled.clearBuffer();
    _oled.sendBuffer();
}

bool DisplayManager::shouldRender(uint32_t nowMs)
{
    if (nowMs - _lastRenderAt < DISPLAY_FRAME_INTERVAL_MS)
    {
        return false;
    }

    _lastRenderAt = nowMs;
    return true;
}

void DisplayManager::setInvert(bool invert)
{
    _oled.sendF("c", invert ? 0x0a7 : 0x0a6);
}

void DisplayManager::render(
    ScreenId screen,
    const AnimationPlayer &animation,
    const MenuManager &menu,
    const TimeSnapshot &time,
    const WeatherData &weather,
    const BatteryStatus &battery,
    const BirthdayScene &birthday,
    WifiScreenState wifiState,
    uint32_t wifiElapsedMs,
    uint32_t nowMs)
{
    _oled.clearBuffer();

    switch (screen)
    {
    case ScreenId::Boot:
        drawBoot(animation);
        break;
    case ScreenId::Menu:
        drawMenu(menu);
        break;
    case ScreenId::Time:
        drawTime(time);
        break;
    case ScreenId::Weather:
        drawWeather(weather);
        break;
    case ScreenId::Birthday:
        drawBirthday(birthday, nowMs);
        break;
    case ScreenId::Wifi:
        drawWifi(wifiState, wifiElapsedMs);
        break;
    case ScreenId::Battery:
        drawBattery(battery);
        break;
    case ScreenId::Domore:
    default:
        drawDomore(animation, time, weather, battery);
        break;
    }

    _oled.sendBuffer();
}

U8G2 &DisplayManager::raw()
{
    return _oled;
}

void DisplayManager::drawDomore(const AnimationPlayer &animation, const TimeSnapshot &time, const WeatherData &weather, const BatteryStatus &battery)
{
    (void)time;
    (void)weather;
    (void)battery;
    animation.draw(_oled);
}

void DisplayManager::drawBoot(const AnimationPlayer &animation)
{
    animation.draw(_oled);
}

void DisplayManager::drawMenu(const MenuManager &menu)
{
    _oled.setFont(u8g2_font_6x10_tf);
    
    // Draw centered title
    const char *title = "Menu";
    int titleW = _oled.getStrWidth(title);
    _oled.drawStr((DISPLAY_WIDTH - titleW) / 2, 9, title);
    _oled.drawHLine(0, 12, DISPLAY_WIDTH);

    const uint8_t visibleRows = 5;
    uint8_t start = 0;
    if (menu.selectedIndex() >= visibleRows)
    {
        start = menu.selectedIndex() - visibleRows + 1;
    }

    // Scrollbar if needed
    if (menu.count() > visibleRows)
    {
        uint8_t barH = (visibleRows * 48) / menu.count();
        if (barH < 4) barH = 4;
        uint8_t maxScroll = 48 - barH;
        uint8_t barY = 14 + (menu.selectedIndex() * maxScroll) / (menu.count() - 1);
        
        _oled.drawRFrame(123, 14, 4, 48, 1);
        _oled.drawRBox(124, barY + 1, 2, barH - 2, 1);
    }

    for (uint8_t row = 0; row < visibleRows && start + row < menu.count(); row++)
    {
        const uint8_t itemIndex = start + row;
        const int16_t y = 24 + row * 10;

        if (itemIndex == menu.selectedIndex())
        {
            _oled.drawRBox(2, y - 8, DISPLAY_WIDTH - 10, 10, 2);
            _oled.setDrawColor(0);
        }

        _oled.drawStr(6, y, menu.itemAt(itemIndex).label);

        if (itemIndex == menu.selectedIndex())
        {
            _oled.setDrawColor(1);
        }
    }
}

void DisplayManager::drawTime(const TimeSnapshot &time)
{
    _oled.setFont(u8g2_font_7x14B_tf);
    drawCentered(time.valid ? time.timeText : "--:--:--", 29);
    _oled.setFont(u8g2_font_6x10_tf);
    drawCentered(time.valid ? time.dateText : "Syncing time", 47);
}

void DisplayManager::drawWeather(const WeatherData &weather)
{
    _oled.setFont(u8g2_font_6x10_tf);
    _oled.drawStr(0, 9, "Weather");
    _oled.drawHLine(0, 12, DISPLAY_WIDTH);

    if (!weather.valid)
    {
        drawCentered("Weather pending", 34);
        drawCentered(weather.summary, 46);
        return;
    }

    drawWeatherIcon(weather, 13, 34);

    char line[24];
    snprintf(line, sizeof(line), "%.1f C", weather.temperatureC);

    _oled.setFont(u8g2_font_7x14B_tf);
    _oled.drawStr(45, 32, line);

    _oled.setFont(u8g2_font_6x10_tf);
    if (weather.humidity > 0)
    {
        snprintf(line, sizeof(line), "%u%%  %s", weather.humidity, weather.summary);
    }
    else
    {
        snprintf(line, sizeof(line), "%s", weather.summary);
    }
    _oled.drawStr(45, 49, line);
}

void DisplayManager::drawWeatherIcon(const WeatherData &weather, int16_t x, int16_t y)
{
    if (weather.weatherCode == 0)
    {
        _oled.drawCircle(x, y, 8);
        _oled.drawLine(x - 13, y, x - 10, y);
        _oled.drawLine(x + 10, y, x + 13, y);
        _oled.drawLine(x, y - 13, x, y - 10);
        _oled.drawLine(x, y + 10, x, y + 13);
        _oled.drawLine(x - 9, y - 9, x - 7, y - 7);
        _oled.drawLine(x + 7, y + 7, x + 9, y + 9);
        _oled.drawLine(x - 9, y + 9, x - 7, y + 7);
        _oled.drawLine(x + 7, y - 7, x + 9, y - 9);
        return;
    }

    _oled.drawDisc(x - 4, y, 6);
    _oled.drawDisc(x + 4, y - 2, 8);
    _oled.drawDisc(x + 11, y + 1, 5);
    _oled.drawBox(x - 9, y, 26, 8);

    if ((weather.weatherCode >= 51 && weather.weatherCode <= 67) ||
        (weather.weatherCode >= 80 && weather.weatherCode <= 82) ||
        weather.weatherCode >= 95)
    {
        _oled.drawLine(x - 6, y + 13, x - 9, y + 18);
        _oled.drawLine(x + 2, y + 13, x - 1, y + 18);
        _oled.drawLine(x + 10, y + 13, x + 7, y + 18);
    }
}

void DisplayManager::drawBirthday(const BirthdayScene &birthday, uint32_t nowMs)
{
    birthday.draw(_oled, nowMs);
}

void DisplayManager::drawWifi(WifiScreenState state, uint32_t elapsedMs)
{
    _oled.setFont(u8g2_font_6x10_tf);
    _oled.drawStr(0, 9, "WIFI");
    _oled.drawHLine(0, 12, DISPLAY_WIDTH);

    _oled.setFont(u8g2_font_6x10_tf);
    if (state == WifiScreenState::Loading)
    {
        uint8_t dots = (elapsedMs / 500) % 4;
        char text[16];
        snprintf(text, sizeof(text), "Connecting");
        for (uint8_t i = 0; i < dots; i++)
        {
            strcat(text, ".");
        }
        drawCentered(text, 36);
    }
    else if (state == WifiScreenState::Connected)
    {
        drawCentered("Connected!", 36);
    }
    else if (state == WifiScreenState::Failed)
    {
        drawCentered("Failed", 30);
        _oled.setFont(u8g2_font_5x8_tf);
        drawCentered("Tap to retry", 44);
        drawCentered("Hold to menu", 54);
    }
}

void DisplayManager::drawStatusBar(const TimeSnapshot &time, const WeatherData &weather)
{
    _oled.setDrawColor(0);
    _oled.drawBox(0, 0, DISPLAY_WIDTH, 10);
    _oled.setDrawColor(1);

    _oled.setFont(u8g2_font_5x8_tf);
    _oled.drawStr(0, 8, time.valid ? time.timeText : "--:--");

    if (weather.valid)
    {
        char text[16];
        snprintf(text, sizeof(text), "%.0fC", weather.temperatureC);
        _oled.drawStr(103, 8, text);
    }
}

void DisplayManager::drawBatteryWarning(const BatteryStatus &battery)
{
    if (!battery.low)
    {
        return;
    }

    _oled.setFont(u8g2_font_5x8_tf);
    _oled.setDrawColor(0);
    _oled.drawBox(78, 0, 50, 10);
    _oled.setDrawColor(1);

    char text[16];
    snprintf(text, sizeof(text), battery.critical ? "BAT %.1f!" : "BAT %.1f", battery.voltage);
    _oled.drawStr(80, 8, text);
}

void DisplayManager::drawBattery(const BatteryStatus &battery)
{
    _oled.setFont(u8g2_font_6x10_tf);
    _oled.drawStr(0, 9, "Battery");
    _oled.drawHLine(0, 12, DISPLAY_WIDTH);

    if (!battery.valid)
    {
        drawCentered("Reading...", 36);
        return;
    }

    // Voltage text
    char voltText[16];
    snprintf(voltText, sizeof(voltText), "%.2f V", battery.voltage);
    _oled.setFont(u8g2_font_7x14B_tf);
    drawCentered(voltText, 30);

    // Battery bar
    const uint8_t barX = 14;
    const uint8_t barY = 36;
    const uint8_t barW = 100;
    const uint8_t barH = 10;
    _oled.drawFrame(barX, barY, barW, barH);
    _oled.drawBox(barX + barW, barY + 2, 3, barH - 4);

    // Map voltage to percentage (3.0V = 0%, 4.2V = 100%)
    float pct = (battery.voltage - 3.0f) / (4.2f - 3.0f) * 100.0f;
    if (pct < 0.0f) pct = 0.0f;
    if (pct > 100.0f) pct = 100.0f;

    uint8_t fillW = (uint8_t)((barW - 4) * pct / 100.0f);
    if (fillW > 0)
    {
        _oled.drawBox(barX + 2, barY + 2, fillW, barH - 4);
    }

    // Status text
    _oled.setFont(u8g2_font_6x10_tf);
    const char *status = battery.critical ? "Critical!" : (battery.low ? "Low" : "Good");
    char line[24];
    snprintf(line, sizeof(line), "%d%% - %s", (int)pct, status);
    drawCentered(line, 58);
}

void DisplayManager::drawCentered(const char *text, int16_t y)
{
    if (text == nullptr)
    {
        return;
    }

    const int16_t width = _oled.getStrWidth(text);
    _oled.drawStr((DISPLAY_WIDTH - width) / 2, y, text);
}
}
