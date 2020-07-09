#include "Clock.h"

#define CLOCK_RADIUS    (Y_CENTER - 9)
#define CLOCK_Y_CENTER  (CLOCK_RADIUS + 2)


void wall_clock_redraw() {
  M5.Lcd.fillScreen(colors[cur_color].bg_color);
  M5.Lcd.drawCircle(X_CENTER, CLOCK_Y_CENTER, CLOCK_RADIUS,   colors[cur_color].fg_color);
  M5.Lcd.drawCircle(X_CENTER, CLOCK_Y_CENTER, CLOCK_RADIUS-1, colors[cur_color].fg_color);

    M5.Lcd.drawCentreString(BUTTON_A_TITLE, BUTTON_A_CENTER, BUTTON_TITLE_LINE, 1);
    M5.Lcd.drawCentreString(BUTTON_B_TITLE, BUTTON_B_CENTER, BUTTON_TITLE_LINE, 1);
    M5.Lcd.drawCentreString(BUTTON_C_TITLE, BUTTON_C_CENTER, BUTTON_TITLE_LINE, 1);
}


void wall_clock() {
  return;
}
