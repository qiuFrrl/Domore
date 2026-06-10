#pragma once

#include <Arduino.h>

namespace robodesk
{
enum class MenuAction : uint8_t
{
    ShowDomore,
    ShowTime,
    ShowWeather,
    ShowWifi,
    ShowBattery
};

struct MenuItem
{
    const char *label;
    MenuAction action;
};

class MenuManager
{
public:
    void begin();
    void next();
    void previous();

    MenuAction select() const;
    const MenuItem &selected() const;
    const MenuItem &itemAt(uint8_t index) const;
    uint8_t selectedIndex() const;
    uint8_t count() const;

private:
    uint8_t _selectedIndex = 0;
};
}
