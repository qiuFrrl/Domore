#pragma once

#include <Arduino.h>

#include "animation/DomoreAnimationManager.h"
#include "display/AnimationCatalog.h"
#include "display/AnimationPlayer.h"
#include "display/BirthdayScene.h"
#include "display/DisplayManager.h"
#include "input/InputManager.h"
#include "menu/MenuManager.h"
#include "system/BatteryManager.h"
#include "system/LocationManager.h"
#include "system/TimeManager.h"
#include "system/WeatherManager.h"
#include "system/WifiManager.h"
#include "system/FirebaseManager.h"

namespace robodesk
{
enum class BootStage : uint8_t
{
    Intro,
    IntroHold,
    Excited,
    BirthdayCheck,
    Done
};

class RobodeskApp
{
public:
    void begin();
    void update();

private:
    void handleInput(InputEvent event);
    void updateBoot(uint32_t nowMs);
    void performMenuAction(MenuAction action);
    void returnDomore(uint32_t nowMs);
    void startBirthday(uint32_t nowMs);
    void updateBirthday(uint32_t nowMs);
    bool shouldStartBirthday() const;
    void updateWifi(uint32_t nowMs);

    DisplayManager _display;
    AnimationPlayer _animation;
    DomoreAnimationManager _domoreAnimations;
    BirthdayScene _birthday;
    InputManager _input;
    MenuManager _menu;
    WifiManager _wifi;
    TimeManager _time;
    WeatherManager _weather;
    LocationManager _location;
    BatteryManager _battery;
    FirebaseManager _firebase;

    ScreenId _screen = ScreenId::Boot;
    BootStage _bootStage = BootStage::Intro;
    uint32_t _bootIntroFinishedAt = 0;
    uint32_t _birthdayCheckStartedAt = 0;
    bool _birthdayPlayedThisBoot = false;
    uint32_t _wifiScreenStartMs = 0;
    bool _wifiFirebaseFetched = false;
    bool _displayInverted = false;
};
}
