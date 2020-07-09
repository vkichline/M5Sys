#include "WallClock.h"



WallClock::WallClock() {
  VERBOSE("WallClock::WallClock()\n");
}


void WallClock::draw_maximum() {
  VERBOSE("WallClock::draw_maximum()\n");
  M5.Lcd.fillScreen(colors[cur_color].bg_color);
  M5.Lcd.drawCircle(X_CENTER, CLOCK_Y_CENTER, CLOCK_RADIUS,   colors[cur_color].fg_color);
  M5.Lcd.drawCircle(X_CENTER, CLOCK_Y_CENTER, CLOCK_RADIUS-1, colors[cur_color].fg_color);
  draw_button_titles();
}


void WallClock::draw_minimum() {
  VERBOSE("WallClock::draw_minimum()\n");
  return;
}
