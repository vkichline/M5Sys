#pragma once

#include <stdint.h>
#include "DiffMeter.h"


class M5Accl {
  public:
    M5Accl();
    void        clear();
    void        show();
    void        zero();
  protected:
    float       x;
    float       y;
    float       z;
    float       zero_offset_x;
    float       zero_offset_y;
    float       zero_offset_z;
    DiffMeter*  x_meter;
    DiffMeter*  y_meter;
    DiffMeter*  z_meter;
};
