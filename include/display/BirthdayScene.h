#pragma once

#include <Arduino.h>
#include <U8g2lib.h>

namespace robodesk
{
class BirthdayScene
{
public:
    void begin(uint32_t nowMs);
    bool isRunning() const;
    bool isFinished(uint32_t nowMs) const;
    void draw(U8G2 &display, uint32_t nowMs) const;

private:
    void drawCentered(U8G2 &display, const char *text, int16_t y) const;
    void drawCake(U8G2 &display, int16_t x, int16_t y) const;
    void drawFirework(U8G2 &display, int16_t x, int16_t y, uint8_t radius) const;
    void drawFlower(U8G2 &display, int16_t x, int16_t y) const;

    uint32_t _startedAt = 0;
    bool _running = false;
};
}
