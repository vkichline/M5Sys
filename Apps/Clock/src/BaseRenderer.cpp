#include <M5SysBase.h>
#include "Clock.h"
#include "BaseRenderer.h"

extern  M5SysBase m5sys;

#define BUTTON_A_CENTER   68
#define BUTTON_B_CENTER   160
#define BUTTON_C_CENTER   252
#define BUTTON_A_TITLE    "Colors"
#define BUTTON_B_TITLE    "TimeZones"
#define BUTTON_C_TITLE    "Faces"


void BaseRenderer::draw_button_titles() {
  VERBOSE("BaseRenderer::draw_button_titles()\n");
  M5.Lcd.drawCentreString(BUTTON_A_TITLE, BUTTON_A_CENTER, BUTTON_TITLE_LINE, 1);
  M5.Lcd.drawCentreString(BUTTON_B_TITLE, BUTTON_B_CENTER, BUTTON_TITLE_LINE, 1);
  M5.Lcd.drawCentreString(BUTTON_C_TITLE, BUTTON_C_CENTER, BUTTON_TITLE_LINE, 1);
}
