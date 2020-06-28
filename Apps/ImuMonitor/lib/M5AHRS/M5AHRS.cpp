#define M5STACK_MPU6886   // define must ahead #include <M5Stack.h>
#include <M5Sys.h>
#include "M5AHRS.h"

#define CROSSHATCH_COLOR  RED
#define OUTER_RETICULE    90
#define INNER_RETICULE    45
#define BUBBLE_DIAMETER   4
#define LOOP_DELAY        10

extern  M5Sys             m5sys;


void M5AHRS::clear() {
  pitch             = 0.0F;
  roll              = 0.0F;
  yaw               = 0.0F;
  zero_offset_pitch = 0.0F;
  zero_offset_roll  = 0.0F;
  ez.canvas.clear();
}


void M5AHRS::show() {
  VERBOSE("M5AHRS::show()\n");
  String  response;
  int16_t  last_x = 500;  // Impossible value, to force first redraw
  int16_t  last_y = 0;

  ez.buttons.show("Home # Accl # Zero");
  ez.header.show("AHRS Bubble Scale");
  clear();
  while(true) {
    M5.IMU.getAhrsData(&pitch, &roll, &yaw);
    // Yaw is of little interest; sampling error is high and it spins like a top
    pitch -= zero_offset_pitch;
    roll  -= zero_offset_roll;
    DEBUG("M5Gyro::show(): pitch = %.3f, roll = %.3f\n", pitch, roll);
    if(last_x != (int16_t)pitch && last_y != (int16_t)yaw) {
      draw_bubble(last_x, last_y, ez.theme->background);
      draw_reticule();
      last_x = (int16_t)pitch;
      last_y = (int16_t)roll;
      draw_bubble(last_x, last_y, CROSSHATCH_COLOR);
    }
    response = ez.buttons.poll();
    if(0 < response.length()) {
      INFO("response = %s\n", response);
      if(     0 == response.compareTo("Home")) m5sys.goHome();
      if(     0 == response.compareTo("Accl")) return;
      else if(0 == response.compareTo("Zero")) zero();
    }
    delay(LOOP_DELAY);
  }
}

void M5AHRS::draw_reticule() {
  VERBOSE("M5AHRS::draw_reticule()\n");
  M5.Lcd.drawCircle(160, 120, OUTER_RETICULE, ez.theme->foreground);
  M5.Lcd.drawCircle(160, 120, INNER_RETICULE, ez.theme->foreground);
  M5.Lcd.drawLine(   70, 120, 250, 120,       ez.theme->foreground);
  M5.Lcd.drawLine(  160,  30, 160, 210,       ez.theme->foreground);
}


// To draw a bubble scale that makes sense to the eye, we have to invert x & y
// The input is in degrees, but we must map it to a reticule that's a little
// smaller that 1:1; at a radius of 90, the bead overwrites the header at extremes.
// Note that, even though limited, the bead can draw 1/2 its diameter outside
// the OUTER_RETICULE.
// The above is no longer true. With a smaller bead, I've expanded the reticule
// radius back to 90, but I leave the math in place, because it's blindingly
// fast to integer math, and it provides the flexibility of chaning the diameter
// of the display
//
void M5AHRS::draw_bubble(int16_t x, int16_t y, uint16_t color) {
  VERBOSE("M5AHRS::draw_bead(%d, %d, %d)\n", x, y, color);
  uint16_t dx     = 160 - limit((x * OUTER_RETICULE) / 90, OUTER_RETICULE);
  uint16_t dy     = 120 - limit((y * OUTER_RETICULE) / 90, OUTER_RETICULE);
  DEBUG("M5AHRS::draw_bead: draw position = %d, %d\n", dx, dy);
  M5.Lcd.fillCircle(dx, dy, BUBBLE_DIAMETER, color);
}


// Limit value to +/- range
int16_t M5AHRS::limit(int16_t value, uint16_t range) {
  value = (value > range)  ? range  :  value;
  value = (value < -range) ? -range : value;
  return value;
}


void M5AHRS::zero() {
  VERBOSE("M5AHRS::zero()\n");
  zero_offset_pitch = pitch;
  zero_offset_roll  = roll;
  DEBUG("M5AHRS::zero: zero_offset_pitch = %.3f, zero_offset_roll = %.3f\n", pitch, roll);
}