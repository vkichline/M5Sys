#include "WallClock.h"

#define CLOCK_RADIUS        (Y_CENTER - 9)
#define CLOCK_Y_CENTER      (CLOCK_RADIUS + 2)
#define SHORT_TICK_OFFSET   (CLOCK_RADIUS - 8)
#define LONG_TICK_OFFSET    (CLOCK_RADIUS - 16)
#define SECOND_HAND_LENGTH  (CLOCK_RADIUS - 20)
#define MINUTE_HAND_LENGTH  (CLOCK_RADIUS - 40)
#define HOUR_HAND_LENGTH    (CLOCK_RADIUS - 60)

WallClock::WallClock() {
  VERBOSE("WallClock::WallClock()\n");
  last_hour_angle   = 0.0;
  last_minute_angle = 0.0;
  last_second_angle = 0.0;
  last_hour         = 0;
  last_minute       = 0;
  last_second       = 0;
}


void WallClock::draw_maximum() {
  M5.Lcd.fillScreen(colors[cur_color].bg_color);
  draw_clockface();
  draw_minimum();
  draw_button_titles();
}


void WallClock::draw_minimum() {
  VERBOSE("WallClock::draw_minimum()\n");
  if(last_second == homeTZ.second()) return;
  uint8_t hour              = homeTZ.hour();
  uint8_t minute            = homeTZ.minute();
  uint8_t second            = homeTZ.second();
  double  hour_angle        = (double)(hour * 3600 + minute * 60 + second) / (double)43200.0 * double(PI * 2.0);
  double  minute_angle      = (double)(minute * 60 + second) / (double)3600.0 * double(PI * 2.0);
  double  second_angle      = (double)second / (double)60.0 * double(PI * 2.0);
  bool    draw_hours        = (hour_angle   - last_hour_angle)   > 0.003; // enough to render a new line
  bool    draw_minutes      = (minute_angle - last_minute_angle) > 0.003; // enough to render a new line
  double  past_hour_angle   = (double)(last_hour * 3600 + last_minute * 60 + last_second) / (double)43200.0 * double(PI * 2.0);
  double  past_minute_angle = (double)(last_minute * 60 + last_second) / (double)3600.0 * double(PI * 2.0);
  last_second_angle         = (double)last_second / (double)60.0 * double(PI * 2.0);
  last_hour                 = hour;
  last_minute               = minute;
  last_second               = second;

  // Erase hands which have moved
  if(draw_hours) {
    last_hour_angle = past_hour_angle;
    draw_hand(last_hour_angle,   HOUR_HAND_LENGTH, colors[cur_color].bg_color);
  }
  if(draw_minutes) {
    last_minute_angle = past_minute_angle;
    draw_hand(last_minute_angle, MINUTE_HAND_LENGTH, colors[cur_color].bg_color);
  }
  draw_hand(last_second_angle, SECOND_HAND_LENGTH, colors[cur_color].bg_color);

  // Draw all three hands (part may have been erased)
  draw_hand(hour_angle,   HOUR_HAND_LENGTH,   colors[cur_color].fg_color);
  draw_hand(minute_angle, MINUTE_HAND_LENGTH, colors[cur_color].fg_color);
  draw_hand(second_angle, SECOND_HAND_LENGTH, colors[cur_color].fg_color);
}


// Draw a clock hand in the given color, from the center to the distance indicated.
// Angle is in radians
//
void WallClock::draw_hand(double angle, uint16_t length, uint16_t color) {
  VERBOSE("WallClock::draw_hand(%d, %d)\n", length, color);
  int     end_x, end_y;
  angle   -= ((double)PI/2.0);
  end_x    = X_CENTER + length * cos(angle);
  end_y    = CLOCK_Y_CENTER + length * sin(angle);
  M5.Lcd.drawLine(X_CENTER, CLOCK_Y_CENTER, end_x, end_y, color);
  VERBOSE("x0=%d, y0=%d, x1=%d, y1=%d, color=%d\n", X_CENTER, CLOCK_Y_CENTER, end_x, end_y, color);
}


// Draw the clock face with ticks for hours and 1/8th hours
//
void WallClock::draw_clockface() {
  VERBOSE("WallClock::draw_clockface()\n");
  M5.Lcd.fillScreen(colors[cur_color].bg_color);
  M5.Lcd.drawCircle(X_CENTER, CLOCK_Y_CENTER, CLOCK_RADIUS,   colors[cur_color].fg_color);
  M5.Lcd.drawCircle(X_CENTER, CLOCK_Y_CENTER, CLOCK_RADIUS-1, colors[cur_color].fg_color);

  for(int i = 0; i < 96; i++) {
    lineToEdge(PI/48.0*i, SHORT_TICK_OFFSET, colors[cur_color].fg_color);    
  }
  for(int i = 0; i < 12; i++) {
    lineToEdge(PI/6.0*i, LONG_TICK_OFFSET, colors[cur_color].fg_color);    
  }
}


// Draw a line from the center (skipping offset pixels) to the edge of the circle.
// Offset the angle so that 12 indicates straight up.
// Angle is expressed in radians, offset is # pixels from center to begin drawing.
//
void WallClock::lineToEdge(double angle, int offset, uint32_t color) {
  int start_x, start_y, end_x, end_y;

  VERBOSE("WallClock::lineToEdge(angle=%d/1000, offset=%d, color=%d\n", (int)(angle*1000), offset, color);
  angle   -= ((double)PI/2.0);
  start_x  = X_CENTER + offset * cos(angle);
  start_y  = CLOCK_Y_CENTER + offset * sin(angle);
  end_x    = X_CENTER + (CLOCK_RADIUS-1) * cos(angle);
  end_y    = CLOCK_Y_CENTER + (CLOCK_RADIUS-1) * sin(angle);
  M5.Lcd.drawLine(start_x, start_y, end_x, end_y, color);
  VERBOSE("x0=%d, y0=%d, x1=%d, y1=%d, color=%d\n", start_x, start_y, end_x, end_y, color);
}
