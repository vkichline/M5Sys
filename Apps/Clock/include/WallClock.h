#pragma once

#include "Clock.h"
#include "BaseRenderer.h"


class WallClock : public BaseRenderer {
  public:
    WallClock();
    void    draw_minimum();  // Call as often as possible, draws only what's needed
    void    draw_maximum();  // Call when settings change to redraw everything
  protected:
    void    draw_hands(bool on_second);
    void    draw_clockface();
    void    draw_hand(double angle, uint16_t length, uint16_t color);
    void    lineToEdge(double angle, int offset, uint32_t color);
    uint8_t last_hour;
    uint8_t last_minute;
    uint8_t last_second;
    double  last_hour_angle;
    double  last_minute_angle;
    double  last_second_angle;
};
