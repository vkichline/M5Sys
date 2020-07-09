#pragma once

#include <M5SysBase.h>
#include <ezTime.h>

#define X_WIDTH           320
#define X_CENTER          160
#define Y_HEIGHT          240
#define Y_CENTER          120
#define BUTTON_TITLE_LINE 230

struct ColorCombo {
  uint16_t  fg_color;
  uint16_t  bg_color;
};

// Globals that can be used by renderers
extern  M5SysBase         m5sys;
extern  Timezone          homeTZ;
extern  const ColorCombo  colors[];
extern  const uint8_t     num_colors;
extern  uint8_t           cur_color;
extern  const String      timezones[];
extern  const uint8_t     num_timezones;
extern  uint8_t           cur_timezone;
