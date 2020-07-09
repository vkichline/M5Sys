#pragma once

#include "Clock.h"
#include "BaseRenderer.h"


class WallClock : public BaseRenderer {
  #define CLOCK_RADIUS    (Y_CENTER - 9)
  #define CLOCK_Y_CENTER  (CLOCK_RADIUS + 2)
  public:
    WallClock();
    void  draw_minimum();  // Call as often as possible, draws only what's needed
    void  draw_maximum();  // Call when settings change to redraw everything
  protected:
};
