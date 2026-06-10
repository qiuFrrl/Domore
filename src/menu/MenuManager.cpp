#include "menu/MenuManager.h"

namespace robodesk
{
namespace
{
const MenuItem MENU_ITEMS[] = {
    {"Domore", MenuAction::ShowDomore},
    {"Time", MenuAction::ShowTime},
    {"Weather", MenuAction::ShowWeather},
    {"WIFI", MenuAction::ShowWifi},
    {"Battery", MenuAction::ShowBattery},
    {"Invert", MenuAction::ToggleInvert},
};
}

void MenuManager::begin()
{
    _selectedIndex = 0;
}

void MenuManager::next()
{
    _selectedIndex = (_selectedIndex + 1) % count();
}

void MenuManager::previous()
{
    if (_selectedIndex == 0)
    {
        _selectedIndex = count() - 1;
        return;
    }

    _selectedIndex--;
}

MenuAction MenuManager::select() const
{
    return selected().action;
}

const MenuItem &MenuManager::selected() const
{
    return itemAt(_selectedIndex);
}

const MenuItem &MenuManager::itemAt(uint8_t index) const
{
    if (index >= count())
    {
        return MENU_ITEMS[0];
    }

    return MENU_ITEMS[index];
}

uint8_t MenuManager::selectedIndex() const
{
    return _selectedIndex;
}

uint8_t MenuManager::count() const
{
    return sizeof(MENU_ITEMS) / sizeof(MENU_ITEMS[0]);
}
}
