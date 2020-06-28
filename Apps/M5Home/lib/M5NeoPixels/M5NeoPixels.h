#pragma once

// NeoPixels specific to the 10 included with M5Stack-Fire

#include <Adafruit_NeoPixel.h>

#define M5STACK_FIRE_NEO_NUM_LEDS 10
#define M5STACK_FIRE_NEO_DATA_PIN 15


class M5NeoPixels {
  public:
    M5NeoPixels();
    void  begin();
    void  clear();
    void  brightness(uint8_t level);
    void  color(uint8_t index, uint32_t color);
    void  left(uint32_t color);
    void  right(uint32_t color);
    void  all(uint32_t color);
    uint32_t np_red;
    uint32_t np_green;
    uint32_t np_blue;
    uint32_t np_white;
    uint32_t np_black;
  private:
    Adafruit_NeoPixel pixels;
};
