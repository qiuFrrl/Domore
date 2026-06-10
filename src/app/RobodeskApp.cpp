#include "app/RobodeskApp.h"

#include "config/AppConfig.h"
#include "display/AnimationCatalog.h"
#include <Preferences.h>

namespace robodesk
{
    void RobodeskApp::begin()
    {
        Serial.println();
        Serial.println("ROBODESK booting");

        _display.begin();
        _input.begin();
        _menu.begin();
        _location.begin();
        _battery.begin();
        _wifi.begin(WIFI_CREDENTIALS, WIFI_CREDENTIAL_COUNT);
        _time.begin();
        _weather.begin();
        _domoreAnimations.begin(_animation);

        _screen = ScreenId::Boot;
        _bootStage = BootStage::Intro;
        _bootIntroFinishedAt = 0;
        _birthdayCheckStartedAt = 0;
        _birthdayPlayedThisBoot = false;
        _wifiScreenStartMs = 0;
        _wifiFirebaseFetched = false;

        Preferences prefs;
        prefs.begin("robodesk", true);
        _displayInverted = prefs.getBool("invert", false);
        prefs.end();
        _display.setInvert(_displayInverted);

        _animation.play(AnimationCatalog::get(AnimationId::Intro), 1, true);
    }

    void RobodeskApp::update()
    {
        const uint32_t nowMs = millis();

        _input.update();
        handleInput(_input.consumeEvent());

        _location.update(nowMs);
        _wifi.update(nowMs);
        _time.update(nowMs);
        _weather.update(nowMs, _wifi.isConnected(), _location.current());
        _battery.update(nowMs);
        _animation.update(nowMs);

        if (_screen == ScreenId::Boot)
        {
            updateBoot(nowMs);
        }

        if (_screen == ScreenId::Domore)
        {
            _domoreAnimations.updateHome(nowMs, _time.snapshot(), _weather.data());
        }
        else if (_screen == ScreenId::Birthday)
        {
            updateBirthday(nowMs);
        }
        else if (_screen == ScreenId::Wifi)
        {
            updateWifi(nowMs);
        }

        if (_display.shouldRender(nowMs))
        {
            WifiScreenState wifiState = WifiScreenState::Loading;
            uint32_t wifiElapsedMs = 0;
            if (_screen == ScreenId::Wifi) {
                wifiElapsedMs = nowMs - _wifiScreenStartMs;
                if (_wifi.isConnected()) {
                    wifiState = (_wifiFirebaseFetched && wifiElapsedMs >= 5000) ? WifiScreenState::Connected : WifiScreenState::Loading;
                } else {
                    wifiState = (wifiElapsedMs >= 10000) ? WifiScreenState::Failed : WifiScreenState::Loading;
                }
            }

            _display.render(
                _screen,
                _animation,
                _menu,
                _time.snapshot(),
                _weather.data(),
                _battery.status(),
                _birthday,
                wifiState,
                wifiElapsedMs,
                nowMs);
        }
    }

    void RobodeskApp::updateBoot(uint32_t nowMs)
    {
        switch (_bootStage)
        {
        case BootStage::Intro:
            if (_animation.isFinished())
            {
                _bootIntroFinishedAt = nowMs;
                _bootStage = BootStage::IntroHold;
            }
            break;

        case BootStage::IntroHold:
            if (nowMs - _bootIntroFinishedAt >= BOOT_INTRO_LAST_FRAME_HOLD_MS)
            {
                _animation.play(AnimationCatalog::get(AnimationId::Excited), 1, true);
                _bootStage = BootStage::Excited;
            }
            break;

        case BootStage::Excited:
            if (_animation.isFinished())
            {
                _birthdayCheckStartedAt = nowMs;
                _bootStage = BootStage::BirthdayCheck;
                _animation.play(AnimationCatalog::get(AnimationId::Blank), 0, true);
            }
            break;

        case BootStage::BirthdayCheck:
            if (!_time.snapshot().valid)
            {
                _time.syncNow();
            }

            if (shouldStartBirthday())
            {
                _bootStage = BootStage::Done;
                startBirthday(nowMs);
            }
            else if (_time.snapshot().valid || nowMs - _birthdayCheckStartedAt >= BIRTHDAY_BOOT_TIME_WAIT_MS)
            {
                _bootStage = BootStage::Done;
                returnDomore(nowMs);
            }
            break;

        case BootStage::Done:
        default:
            break;
        }
    }

    void RobodeskApp::handleInput(InputEvent event)
    {
        if (event.type == InputEventType::None || _screen == ScreenId::Boot || _screen == ScreenId::Birthday)
        {
            return;
        }

        const uint32_t nowMs = millis();

        if (_screen == ScreenId::Menu)
        {
            if (event.type == InputEventType::TapSequence)
            {
                _menu.next();
            }
            else if (event.type == InputEventType::Hold || event.type == InputEventType::MenuHold)
            {
                performMenuAction(_menu.select());
            }
            return;
        }

        if (_screen == ScreenId::Wifi)
        {
            if (event.type == InputEventType::TapSequence)
            {
                _wifi.forceConnect();
                _wifiScreenStartMs = nowMs;
                _wifiFirebaseFetched = false;
            }
            else if (event.type == InputEventType::MenuHold)
            {
                _domoreAnimations.stopHome();
                _screen = ScreenId::Menu;
            }
            return;
        }

        if (event.type == InputEventType::MenuHold)
        {
            _domoreAnimations.stopHome();
            _screen = ScreenId::Menu;
            return;
        }

        if (_screen != ScreenId::Domore)
        {
            return;
        }

        if (event.type == InputEventType::TapSequence)
        {
            if (_domoreAnimations.isInactive())
            {
                _domoreAnimations.playInactiveTapWake(nowMs);
            }
            else
            {
                _domoreAnimations.playTapSequence(event.tapCount, nowMs);
            }
        }
        else if (event.type == InputEventType::Hold)
        {
            if (_domoreAnimations.isInactive())
            {
                _domoreAnimations.playInactiveHoldWake(nowMs);
            }
            else
            {
                _domoreAnimations.playHold(nowMs);
            }
        }
    }

    void RobodeskApp::performMenuAction(MenuAction action)
    {
        switch (action)
        {
        case MenuAction::ShowTime:
            _domoreAnimations.stopHome();
            _animation.stop();
            _screen = ScreenId::Time;
            break;
        case MenuAction::ShowWeather:
            _domoreAnimations.stopHome();
            _animation.stop();
            _screen = ScreenId::Weather;
            break;
        case MenuAction::ShowWifi:
            _domoreAnimations.stopHome();
            _animation.stop();
            _screen = ScreenId::Wifi;
            _wifiScreenStartMs = millis();
            _wifiFirebaseFetched = false;
            if (!_wifi.isConnected())
            {
                _wifi.forceConnect();
            }
            break;
        case MenuAction::ShowBattery:
            _domoreAnimations.stopHome();
            _animation.stop();
            _screen = ScreenId::Battery;
            break;
        case MenuAction::ToggleInvert:
        {
            _displayInverted = !_displayInverted;
            _display.setInvert(_displayInverted);
            
            Preferences prefs;
            prefs.begin("robodesk", false);
            prefs.putBool("invert", _displayInverted);
            prefs.end();
            break;
        }
        case MenuAction::ShowDomore:
        default:
            returnDomore(millis());
            break;
        }
    }

    void RobodeskApp::updateWifi(uint32_t nowMs)
    {
        uint32_t elapsed = nowMs - _wifiScreenStartMs;
        if (_wifi.isConnected() && !_wifiFirebaseFetched && elapsed > 100)
        {
            _firebase.fetchWifi(_wifi);
            _wifiFirebaseFetched = true;
        }
    }

    void RobodeskApp::returnDomore(uint32_t nowMs)
    {
        _screen = ScreenId::Domore;
        _domoreAnimations.startHome(nowMs);
    }

    void RobodeskApp::startBirthday(uint32_t nowMs)
    {
        _domoreAnimations.stopHome();
        _animation.stop();
        _birthday.begin(nowMs);
        _birthdayPlayedThisBoot = true;
        _screen = ScreenId::Birthday;
    }

    void RobodeskApp::updateBirthday(uint32_t nowMs)
    {
        if (!_birthday.isFinished(nowMs))
        {
            return;
        }

        returnDomore(nowMs);
        _domoreAnimations.playBirthdayFinal(nowMs);
    }

    bool RobodeskApp::shouldStartBirthday() const
    {
        const TimeSnapshot &time = _time.snapshot();
        return time.valid &&
               time.day == BIRTHDAY_DAY &&
               time.month == BIRTHDAY_MONTH &&
               !_birthdayPlayedThisBoot;
    }
}
