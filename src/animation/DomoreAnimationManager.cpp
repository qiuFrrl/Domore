#include "animation/DomoreAnimationManager.h"

#include <esp_system.h>

#include "animation/AnimationSets.h"
#include "config/AppConfig.h"
#include "display/AnimationCatalog.h"

namespace robodesk
{
    namespace
    {
        constexpr uint8_t MAX_HOME_CANDIDATES = 11;
    }

    void DomoreAnimationManager::begin(AnimationPlayer &player)
    {
        _player = &player;
        randomSeed(esp_random());
    }

    void DomoreAnimationManager::startHome(uint32_t nowMs)
    {
        _homeEnabled = true;
        _inactive = false;
        _waitingForHomeAnimationToFinish = false;
        _holdBlankUntilNextInput = false;
        _lastHomeAnimation = AnimationId::Count;
        _lastButtonActivityAt = nowMs;
        _homeStep = 0;
        _nextHomeAnimationAt = nowMs;
    }

    void DomoreAnimationManager::stopHome()
    {
        _homeEnabled = false;
        _waitingForHomeAnimationToFinish = false;
        _holdBlankUntilNextInput = false;
    }

    void DomoreAnimationManager::updateHome(
        uint32_t nowMs,
        const TimeSnapshot &time,
        const WeatherData &weather,
        const BatteryStatus &battery)
    {
        if (!_homeEnabled || _player == nullptr)
        {
            return;
        }

        if (_holdBlankUntilNextInput)
        {
            const bool playingBlank = _player->currentClip() == &AnimationCatalog::get(AnimationId::Blank);
            if (playingBlank)
            {
                _holdBlankUntilNextInput = false;
                _waitingForHomeAnimationToFinish = false;
                scheduleNextHome(nowMs);
            }
            return;
        }

        if (!_inactive && nowMs - _lastButtonActivityAt >= BUTTON_INACTIVITY_SLEEP_MS)
        {
            _inactive = true;
            _waitingForHomeAnimationToFinish = false;
            playLoop(AnimationSets::inactiveAnimation());
            return;
        }

        if (_inactive)
        {
            return;
        }

        const bool playingBlank = _player->currentClip() == &AnimationCatalog::get(AnimationId::Blank);
        if (_player->isRunning() && !playingBlank)
        {
            return;
        }

        if (_waitingForHomeAnimationToFinish)
        {
            _waitingForHomeAnimationToFinish = false;
            playLoop(AnimationId::Blank);
            scheduleNextHome(nowMs);
            return;
        }

        if (battery.valid && battery.low)
        {
            const bool neverPlayedLowBattery = _lastLowBatteryAnimationAt == 0;
            const bool reminderDue = nowMs - _lastLowBatteryAnimationAt >= LOW_BATTERY_ANIMATION_REMINDER_MS;
            if (neverPlayedLowBattery || reminderDue)
            {
                _lastLowBatteryAnimationAt = nowMs;
                playOneShot(AnimationSets::lowBatteryAnimation());
                return;
            }
        }

        if (nowMs < _nextHomeAnimationAt)
        {
            return;
        }

        const AnimationId nextAnimation = nextHomeAnimation(time, weather);
        _lastHomeAnimation = nextAnimation;
        playOneShot(nextAnimation);
    }

    bool DomoreAnimationManager::isInactive() const
    {
        return _inactive;
    }

    void DomoreAnimationManager::playTapSequence(uint8_t tapCount, uint32_t nowMs)
    {
        recordButtonActivity(nowMs);
        _holdBlankUntilNextInput = true;
        playOneShot(AnimationSets::tapAnimationForCount(tapCount));
    }

    void DomoreAnimationManager::playHold(uint32_t nowMs)
    {
        recordButtonActivity(nowMs);
        playOneShot(AnimationSets::holdAnimation());
    }

    void DomoreAnimationManager::playInactiveTapWake(uint32_t nowMs)
    {
        recordButtonActivity(nowMs);
        playOneShot(AnimationSets::inactiveTapWakeAnimation());
    }

    void DomoreAnimationManager::playInactiveHoldWake(uint32_t nowMs)
    {
        recordButtonActivity(nowMs);
        playOneShot(AnimationSets::inactiveHoldWakeAnimation());
    }

    void DomoreAnimationManager::playBirthdayFinal(uint32_t nowMs)
    {
        recordButtonActivity(nowMs);
        playOneShot(AnimationSets::birthdayFinalAnimation());
    }

    AnimationId DomoreAnimationManager::nextHomeAnimation(const TimeSnapshot &time, const WeatherData &weather)
    {
        AnimationId candidates[MAX_HOME_CANDIDATES];
        uint8_t count = 0;

        uint8_t defaultCount = 0;
        const AnimationId *defaults = AnimationSets::defaultAnimations(defaultCount);
        for (uint8_t i = 0; i < defaultCount && count < MAX_HOME_CANDIDATES; i++)
        {
            candidates[count++] = defaults[i];
        }

        AnimationSets::appendTimeAnimations(time, candidates, count, MAX_HOME_CANDIDATES);
        AnimationSets::appendWeatherAnimations(weather, candidates, count, MAX_HOME_CANDIDATES);

        if (count == 0)
        {
            return AnimationId::Smile;
        }

        for (uint8_t attempt = 0; attempt < 4; attempt++)
        {
            const AnimationId candidate = candidates[random(count)];
            if (count == 1 || candidate != _lastHomeAnimation)
            {
                _homeStep++;
                return candidate;
            }
        }

        const AnimationId selected = candidates[random(count)];
        _homeStep++;
        return selected;
    }

    void DomoreAnimationManager::playOneShot(AnimationId animation)
    {
        if (_player == nullptr)
        {
            return;
        }

        _player->play(AnimationCatalog::get(animation), 1, true);
        _waitingForHomeAnimationToFinish = _homeEnabled;
    }

    void DomoreAnimationManager::playLoop(AnimationId animation)
    {
        if (_player == nullptr)
        {
            return;
        }

        _player->play(AnimationCatalog::get(animation), 0, true);
    }

    void DomoreAnimationManager::scheduleNextHome(uint32_t nowMs)
    {
        const uint32_t delayMs = random(HOME_RANDOM_DELAY_MIN_MS, HOME_RANDOM_DELAY_MAX_MS + 1UL);
        _nextHomeAnimationAt = nowMs + delayMs;
    }

    void DomoreAnimationManager::recordButtonActivity(uint32_t nowMs)
    {
        _inactive = false;
        _holdBlankUntilNextInput = false;
        _lastButtonActivityAt = nowMs;
    }
}
