#pragma once

#include "Clock.h"
#include "BaseRenderer.h"


class TextClock : public BaseRenderer {
  public:
    TextClock();
    void      draw_minimum();  // Call as often as possible, draws only what's needed
    void      draw_maximum();  // Call when settings change to redraw everything
  protected:
    bool      first_time = true;
    uint8_t   last_day   = 0;
    int16_t   time_width = 0;
};
