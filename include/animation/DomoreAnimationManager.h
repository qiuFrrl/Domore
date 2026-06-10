#pragma once

#include <Arduino.h>

#include "display/AnimationCatalog.h"
#include "display/AnimationPlayer.h"
#include "system/TimeManager.h"
#include "system/WeatherManager.h"

namespace robodesk
{
  class DomoreAnimationManager
  {
  public:
    void begin(AnimationPlayer &player);
    void startHome(uint32_t nowMs);
    void stopHome();
    void updateHome(uint32_t nowMs, const TimeSnapshot &time,
                    const WeatherData &weather);

    bool isInactive() const;
    void playTapSequence(uint8_t tapCount, uint32_t nowMs);
    void playHold(uint32_t nowMs);
    void playInactiveTapWake(uint32_t nowMs);
    void playInactiveHoldWake(uint32_t nowMs);
    void playBirthdayFinal(uint32_t nowMs);

  private:
    AnimationId nextHomeAnimation(const TimeSnapshot &time,
                                  const WeatherData &weather);
    void playOneShot(AnimationId animation);
    void playLoop(AnimationId animation);
    void scheduleNextHome(uint32_t nowMs);
    void recordButtonActivity(uint32_t nowMs);

    AnimationPlayer *_player = nullptr;
    AnimationId _lastHomeAnimation = AnimationId::Count;
    uint32_t _nextHomeAnimationAt = 0;
    uint32_t _lastButtonActivityAt = 0;
    uint8_t _homeStep = 0;
    bool _homeEnabled = false;
    bool _inactive = false;
    bool _waitingForHomeAnimationToFinish = false;
    bool _holdBlankUntilNextInput = false;
  };
}
