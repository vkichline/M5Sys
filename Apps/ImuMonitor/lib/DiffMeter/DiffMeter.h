#pragma once
#include <stdint.h>

// A DiffMeter is like a progress bar, except that zero is in the center
// and there are equal extents of + and -.
// It's described by its rectangle, max_value, and value.
// If the bar is wider than it is tall, it is drawn horizontally.
// If it's taller than wide, it's drawn vertically.

class DiffMeter {
  public:
    DiffMeter(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t max);
    void set_value(int16_t value);
    void draw();
  private:
    uint16_t  x;
    uint16_t  y;
    uint16_t  width;
    uint16_t  height;
    uint16_t  max;
    int16_t   value;
};
