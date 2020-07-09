#pragma once

#include <M5SysBase.h>
#include <ezTime.h>

#define BUTTON_A_CENTER   68
#define BUTTON_B_CENTER   160
#define BUTTON_C_CENTER   254
#define X_WIDTH           320
#define X_CENTER          160


struct ColorCombo {
  uint16_t  fg_color;
  uint16_t  bg_color;
};

typedef void  (*Renderer)(void);

struct RenderCombo {
  Renderer    periodic;   // Call as often as possible, draws only what's needed
  Renderer    redraw;     // Call when settings change to redraw everything
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

// Renderers, defined in separate cpp files in <src>
void text_clock();        // From TextClock.cpp: call periodically
void text_clock_redraw(); // Call when settings are changed to update all fields.