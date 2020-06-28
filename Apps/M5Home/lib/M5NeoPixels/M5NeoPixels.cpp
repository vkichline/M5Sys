#include <M5Sys.h>
#include <Adafruit_NeoPixel.h>
#include "M5NeoPixels.h"

extern M5Sys    m5sys;

M5NeoPixels::M5NeoPixels() {
  VERBOSE("NeoPixels::NeoPixels()\n");
  pixels    = Adafruit_NeoPixel(M5STACK_FIRE_NEO_NUM_LEDS, M5STACK_FIRE_NEO_DATA_PIN, NEO_GRB + NEO_KHZ800);
  np_red    = Adafruit_NeoPixel::Color(255,    0,  0);
  np_green  = Adafruit_NeoPixel::Color(  0,  255,  0);
  np_blue   = Adafruit_NeoPixel::Color(  0,   0, 255);
  np_white  = Adafruit_NeoPixel::Color(255, 255, 255);
  np_black  = Adafruit_NeoPixel::Color(  0,   0,   0);
}


void M5NeoPixels::begin() {
  VERBOSE("NeoPixels::begin()\n");
  pixels.begin();
}


void M5NeoPixels::clear() {
  VERBOSE("NeoPixels::clear()\n");
  all(0); // strangely, pixels.clear() leaves one green pixel on
  pixels.show();
}


void M5NeoPixels::brightness(uint8_t level) {
  VERBOSE("M5NeoPixels::brightness(%d)\n", level);
  pixels.setBrightness(level);
  pixels.show();
}


void M5NeoPixels::color(uint8_t index, uint32_t color) {
  VERBOSE("M5NeoPixels::color(%d, %d)\n", index, color);
  pixels.setPixelColor(index, color);
  pixels.show();
}


void M5NeoPixels::left(uint32_t c) {
  VERBOSE("M5NeoPixels::left(%d)\n", c);
  for(int i = 5; i < 10; i++) {
    color(i, c);
  }
}


void M5NeoPixels::right(uint32_t c) {
  VERBOSE("M5NeoPixels::right(%d)\n", c);
  for(int i = 0; i < 5; i++) {
    color(i, c);
  }
}


void M5NeoPixels::all(uint32_t c) {
  VERBOSE("M5NeoPixels::all(%d)\n", c);
  for(int i = 0; i < 10; i++) {
    color(i, c);
  }
}
