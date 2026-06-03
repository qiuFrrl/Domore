#include "display/BirthdayScene.h"

#include "config/AppConfig.h"

namespace robodesk
{
void BirthdayScene::begin(uint32_t nowMs)
{
    _startedAt = nowMs;
    _running = true;
}

bool BirthdayScene::isRunning() const
{
    return _running;
}

bool BirthdayScene::isFinished(uint32_t nowMs) const
{
    return _running && nowMs - _startedAt >= BIRTHDAY_SCENE_MS;
}

void BirthdayScene::draw(U8G2 &display, uint32_t nowMs) const
{
    const uint32_t elapsed = _running ? nowMs - _startedAt : 0;
    const uint8_t beat = (elapsed / 250) % 4;
    const uint8_t fireworkRadius = 5 + beat * 2;

    display.setFont(u8g2_font_6x10_tf);
    drawCentered(display, "HAPPY", 9);
    display.setFont(u8g2_font_7x14B_tf);
    drawCentered(display, "BIRTHDAY", 24);

    drawCake(display, 44, 38);
    drawFirework(display, 18, 21, fireworkRadius);
    drawFirework(display, 110, 18, 8 + ((beat + 2) % 4));
    drawFlower(display, 18, 54);
    drawFlower(display, 108, 54);

    display.setFont(u8g2_font_5x8_tf);
    drawCentered(display, "7 OCTOBER", 63);
}

void BirthdayScene::drawCentered(U8G2 &display, const char *text, int16_t y) const
{
    const int16_t width = display.getStrWidth(text);
    display.drawStr((DISPLAY_WIDTH - width) / 2, y, text);
}

void BirthdayScene::drawCake(U8G2 &display, int16_t x, int16_t y) const
{
    display.drawFrame(x, y + 9, 40, 12);
    display.drawBox(x + 2, y + 11, 36, 8);
    display.setDrawColor(0);
    display.drawHLine(x + 5, y + 15, 8);
    display.drawHLine(x + 18, y + 15, 8);
    display.drawHLine(x + 31, y + 15, 5);
    display.setDrawColor(1);

    display.drawFrame(x + 6, y + 2, 28, 9);
    display.drawVLine(x + 12, y - 3, 5);
    display.drawVLine(x + 20, y - 3, 5);
    display.drawVLine(x + 28, y - 3, 5);
    display.drawPixel(x + 12, y - 5);
    display.drawPixel(x + 20, y - 5);
    display.drawPixel(x + 28, y - 5);
}

void BirthdayScene::drawFirework(U8G2 &display, int16_t x, int16_t y, uint8_t radius) const
{
    display.drawCircle(x, y, radius);
    display.drawLine(x - radius - 2, y, x - 2, y);
    display.drawLine(x + 2, y, x + radius + 2, y);
    display.drawLine(x, y - radius - 2, x, y - 2);
    display.drawLine(x, y + 2, x, y + radius + 2);
    display.drawLine(x - radius, y - radius, x - 3, y - 3);
    display.drawLine(x + 3, y + 3, x + radius, y + radius);
    display.drawLine(x - radius, y + radius, x - 3, y + 3);
    display.drawLine(x + 3, y - 3, x + radius, y - radius);
}

void BirthdayScene::drawFlower(U8G2 &display, int16_t x, int16_t y) const
{
    display.drawCircle(x, y, 2);
    display.drawDisc(x - 4, y, 2);
    display.drawDisc(x + 4, y, 2);
    display.drawDisc(x, y - 4, 2);
    display.drawDisc(x, y + 4, 2);
    display.drawVLine(x, y + 6, 6);
    display.drawLine(x, y + 9, x - 4, y + 7);
    display.drawLine(x, y + 10, x + 4, y + 8);
}
}
