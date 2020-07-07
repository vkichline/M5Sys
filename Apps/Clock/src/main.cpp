// A simple (but free to grow complex) Clock display program using
// ezTime, which is included in M5ez.

#include <M5SysBase.h>
#include <ezTime.h>

#define   FG_COLOR  WHITE 
#define   BG_COLOR  0x0004

M5SysBase m5sys;
Timezone  homeTZ;


// The workhorse routine; draw as much of the time as needed directly to the screen.
// Flicker-free fonts make buffering unnecessary
//
void draw_time() {
  VERBOSE("draw_time()\n");
  static bool     first_time  = true;
  static uint8_t  last_day    = 0;
  static int16_t  time_width  = 0;
  bool            seconds_too = false;  // because of how secondChanged & minuteChanged work, only one works

  // Draw the hours and the minutes once a minute
  if(first_time || minuteChanged()) {
    DEBUG("updating hours/minutes\n");
    String str = homeTZ.dateTime("g:i");
    // Special case, when time changes from 12:59:59 to 1:00:00, width shrinks and erase is needed
    if(0 == str.compareTo("1:00")) {
      M5.Lcd.fillRect(0, 20, 320, 80, BG_COLOR);
    }
    time_width = M5.Lcd.textWidth(str, 8);
    VERBOSE("time_width = %d\n", time_width);
    M5.Lcd.drawCentreString(str, 130, 20, 8);
    seconds_too = true;   // secondChanged won't trigger after minuteChanged did.
  }

  // Draw the seconds whenever seconds change.
  if(first_time || seconds_too || secondChanged()) {
    DEBUG("updating seconds\n");
    seconds_too = false;
    String str  = homeTZ.dateTime("s");
    M5.Lcd.drawString(str, 140 + time_width / 2, 20, 6);
    str = homeTZ.dateTime("A");
    M5.Lcd.drawString(str, 140 + time_width / 2, 70, 4);
  }

  // Draw the day specific info only once a day.
  if(first_time || day() != last_day) {
    last_day = day();
    String str = homeTZ.dateTime("l  F jS, Y");
    M5.Lcd.drawCentreString(str, 160, 120, 4);
    str = homeTZ.dateTime("e (T)");
    M5.Lcd.drawCentreString(str, 160, 170, 2);
    str = homeTZ.dateTime("~D~a~y z   ~W~e~e~k W");
    M5.Lcd.drawCentreString(str, 160, 194, 2);
  }
  if(first_time) first_time = false;
}


void setup() {
  m5sys.begin("Clock", NETWORK_CONNECTION_AUTO);
  M5.Lcd.setTextColor(FG_COLOR, BG_COLOR);
  M5.Lcd.setTextFont(2);
  M5.Lcd.clearDisplay(BG_COLOR);
  M5.Lcd.print("Connecting ");
  m5sys.wait_for_wifi();
  DEBUG("waiting for sync...\n");
  waitForSync();
  DEBUG("synchronized\n");
  if(!homeTZ.setCache(0)) {
    DEBUG("Doing NTP lookup on timezone info\n");
    homeTZ.setLocation("America/Los_Angeles");
  }
  homeTZ.setDefault();
  M5.Lcd.clearDisplay(BG_COLOR);
}


void loop() {
  draw_time();
  delay(250); // Call more than once per second for higher precision
}
