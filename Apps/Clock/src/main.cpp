// A simple (but free to grow complex) Clock display program using
// ezTime, which is included in M5ez.

// I had to modify ezTime.h, lines 23 & 24, commenting out '#define EZTIME_CACHE_EEPROM'
// and uncommenting #define EZTIME_CACHE_NVS. YOu'll have to do the same.

#include "Clock.h"
#include <Preferences.h>

#define           TZ_CACHE_NAME     "TZCache"

M5SysBase         m5sys;
Timezone          homeTZ;

const ColorCombo  colors[]          = { {WHITE, 0x0004}, {RED, BLACK}, {BLACK, WHITE} };
const uint8_t     num_colors        = sizeof(colors) / sizeof(ColorCombo);
uint8_t           cur_color         = 0;

const String      timezones[]       = { "America/New_York", "America/Chicago", "America/Denver", "America/Los_Angeles", "America/Anchorage", "Pacific/Honolulu" };
const uint8_t     num_timezones     = sizeof(timezones) / sizeof(String);
uint8_t           cur_timezone      = 3;      // Because I happen to live on the West Coast

const RenderCombo renderers[]       = { {text_clock, text_clock_redraw}, {wall_clock, wall_clock_redraw} };
const uint8_t     num_renderers     = sizeof(renderers) / sizeof(RenderCombo);
uint8_t           cur_renderer      = 0;

void              (*renderer)()     = renderers[cur_renderer].periodic;


// Set the colors, timezone (if synchronized) and renderer.
// Call this every time settings are changed.
//
void use_settings() {
  M5.Lcd.setTextColor(colors[cur_color].fg_color, colors[cur_color].bg_color);
  M5.Lcd.clearDisplay(colors[cur_color].bg_color);
  renderer = renderers[cur_renderer].periodic;
  // If we've completed waitForSync()...
  if(timeNotSet != timeStatus()) {
    // The key for the cache name is limited to 15 characters. I think the last 15 will be most unique
    String tz_key = timezones[cur_timezone];
    if(15 < tz_key.length()) tz_key = tz_key.substring(tz_key.length() - 15);
    if(!homeTZ.setCache(TZ_CACHE_NAME, tz_key)) {
      DEBUG("Doing NTP lookup on timezone info for %s\n", timezones[cur_timezone].c_str());
      homeTZ.setLocation(timezones[cur_timezone]);
    }
    homeTZ.setDefault();
  }
}


// Check to see if the A, B or C buttons have been pressed.
// If A, loop through the list of colors to change color settings
// If B, loop through the list of timezones to change the current timezone
// If C, loop through the list of renderers to change the drawing routine
//
void check_for_buttons() {
  VERBOSE("check_for_buttons()\n");
  bool  changed = false;
  M5.update();
  if(M5.BtnA.wasPressed()) {
    cur_color = (++cur_color >= num_colors) ? 0 : cur_color;
    DEBUG("changing cur_color to %d\n", cur_color);
    changed = true;
  }
  if(M5.BtnB.wasPressed()) {
    cur_timezone = (++cur_timezone >= num_timezones) ? 0 : cur_timezone;
    DEBUG("changing cur_timezone to %d\n", cur_timezone);
    changed = true;
  }
  if(M5.BtnC.wasPressed()) {
    cur_renderer = (++cur_renderer >= num_renderers) ? 0 : cur_renderer;
    DEBUG("changing cur_renderer to %d\n", cur_renderer);
    changed = true;
  }
  if(changed) {
    use_settings();
    renderers[cur_renderer].redraw();
  }
}


void setup() {
  m5sys.begin("Clock", NETWORK_CONNECTION_AUTO);
  M5.Lcd.setTextFont(2);
  use_settings();
  M5.Lcd.print("Connecting ");
  m5sys.wait_for_wifi();
  DEBUG("waiting for sync...\n");
  waitForSync();
  DEBUG("synchronized\n");
  use_settings();
}


void loop() {
  check_for_buttons();
  renderer();
  delay(250); // Call more than once per second for higher precision
}
