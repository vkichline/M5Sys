#pragma once

#include "Clock.h"
#include "BaseRenderer.h"


class TextClock : public BaseRenderer {
  #define   Y_LINE_1  20
  #define   Y_LINE_1A 70
  #define   Y_LINE_2  120
  #define   Y_LINE_3  160
  #define   Y_LINE_4  184
  public:
    TextClock();
    void      draw_minimum();  // Call as often as possible, draws only what's needed
    void      draw_maximum();  // Call when settings change to redraw everything
  protected:
    bool      first_time = true;
    uint8_t   last_day   = 0;
    int16_t   time_width = 0;
};
