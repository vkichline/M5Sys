#include "Clock.h"

#define   Y_LINE_1    20
#define   Y_LINE_1A   70
#define   Y_LINE_2    120
#define   Y_LINE_3    170
#define   Y_LINE_4    194
#define   X_WIDTH     320
#define   X_CENTER    160

bool     first_time   = true;
uint8_t  last_day     = 0;
int16_t  time_width   = 0;


// When settings are changed, call this routine to redraw everything
//
void text_clock_redraw() {
  first_time  = true;
  last_day    = 0;
  time_width  = 0;
  text_clock();
}


// The workhorse routine; draw as much of the time as needed directly to the screen.
// Flicker-free fonts make buffering unnecessary
//
void text_clock() {
  VERBOSE("text_clock()\n");
  bool seconds_too = false;  // because of how secondChanged & minuteChanged work, only one works

  // Draw the hours and the minutes once a minute
  if(first_time || minuteChanged()) {
    DEBUG("updating hours/minutes\n");
    String str = homeTZ.dateTime("g:i");
    // Special case, when time changes from 12:59:59 to 1:00:00, or from 9:59:59 to 10:00:00,
    // display width changes and erase is needed. It could be smaller. Needed?
    int16_t new_wid = M5.Lcd.textWidth(str, 8);
    if(time_width  != new_wid) {
      DEBUG("Erasing time background\n");
      M5.Lcd.fillRect(0, Y_LINE_1, X_WIDTH, 80, colors[cur_color].bg_color);
    }
    time_width = new_wid;
    VERBOSE("time_width = %d\n", time_width);
    M5.Lcd.drawCentreString(str, 130, Y_LINE_1, 8);
    seconds_too = true;   // Necessary hack; secondChanged won't trigger after minuteChanged did.
  }

  // Draw the seconds whenever seconds change.
  if(first_time || seconds_too || secondChanged()) {
    DEBUG("updating seconds\n");
    seconds_too = false;
    String str  = homeTZ.dateTime("s");
    M5.Lcd.drawString(str, 140 + time_width / 2, Y_LINE_1, 6);
    str = homeTZ.dateTime("A");
    M5.Lcd.drawString(str, 148 + time_width / 2, Y_LINE_1A, 4);
  }

  // Draw the day specific info only once a day.
  if(first_time || day() != last_day) {
    DEBUG("updating day\n");
    last_day = day();
    String str = homeTZ.dateTime("l  F jS");
    M5.Lcd.drawCentreString(str, X_CENTER, Y_LINE_2, 4);
    str = homeTZ.dateTime("e (T)");
    M5.Lcd.drawCentreString(str, X_CENTER, Y_LINE_3, 2);
    str = homeTZ.dateTime("~D~a~y z   ~W~e~e~k W");
    M5.Lcd.drawCentreString(str, X_CENTER, Y_LINE_4, 2);
  }
  if(first_time) first_time = false;
}
