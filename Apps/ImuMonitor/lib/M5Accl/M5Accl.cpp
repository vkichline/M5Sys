#define M5STACK_MPU6886   // define must ahead #include <M5Stack.h>
#include <M5Sys.h>
#include "M5Accl.h"

extern M5Sys    m5sys;

#define SCREEN_WIDTH    320
#define SCREEN_HEIGHT   240
#define TOP_EXCLUSION   20
#define BOT_EXCLUSION   20
#define MARGIN          24
#define YBAR_X          MARGIN
#define YBAR_Y          (TOP_EXCLUSION + MARGIN)
#define YBAR_H          (SCREEN_HEIGHT - TOP_EXCLUSION - BOT_EXCLUSION - (2 * MARGIN))
#define YBAR_W          80
#define XBAR_X          (YBAR_W + (2 * MARGIN))
#define XBAR_Y          (TOP_EXCLUSION + MARGIN)
#define XBAR_W          (SCREEN_WIDTH - YBAR_W - (3 * MARGIN))
#define XBAR_H          ((SCREEN_HEIGHT - TOP_EXCLUSION - BOT_EXCLUSION - (3 * MARGIN)) / 2)
#define ZBAR_X          XBAR_X
#define ZBAR_Y          (XBAR_Y + XBAR_H + MARGIN)
#define ZBAR_W          XBAR_W
#define ZBAR_H          XBAR_H



M5Accl::M5Accl() {
  VERBOSE("M5Accl::M5Accl()\n");
  clear();
  x_meter = new DiffMeter(XBAR_X, XBAR_Y, XBAR_W,  XBAR_H, 100);
  y_meter = new DiffMeter(YBAR_X, YBAR_Y, YBAR_W,  YBAR_H, 100);
  z_meter = new DiffMeter(ZBAR_X, ZBAR_Y, ZBAR_W,  ZBAR_H, 100);
}


void M5Accl::clear() {
  VERBOSE("M5Accl::clear()\n");
  x             = 0.0F;
  y             = 0.0F;
  z             = 0.0F;
  zero_offset_x = 0.0F;
  zero_offset_y = 0.0F;
  zero_offset_z = 0.0F;
  ez.canvas.clear();
}


void M5Accl::show() {
  VERBOSE("M5Accl::show()\n");
  String response;
  ez.buttons.show("Home # AHRS # Zero");
  ez.header.show("Accelerometer");
  clear();
  while(true) {
    M5.IMU.getAccelData(&x, &y, &z);
    DEBUG("Raw values: x = %f, y = %f, z = %f\n", x, y, z);
    x_meter->set_value((x - zero_offset_x) * 100);
    y_meter->set_value((y - zero_offset_y) * 100);
    z_meter->set_value((z - zero_offset_z) * 100);
    x_meter->draw();
    y_meter->draw();
    z_meter->draw();

    response = ez.buttons.poll();
    if(0 < response.length()) {
      INFO("response = %s\n", response);
      if(     0 == response.compareTo("Home")) m5sys.goHome();
      else if(0 == response.compareTo("AHRS")) return;
      else if(0 == response.compareTo("Zero")) zero();

    }
    delay(10);
  }
}


void M5Accl::zero() {
  VERBOSE("M5Accl::show()\n");
  zero_offset_x = x;
  zero_offset_y = y;
  zero_offset_z = z - 1.0F;
  INFO("M5Accl::zero(): offsets set to x = %.3f, y = %.3f, z = %.3f\n", x, y, z);
}
