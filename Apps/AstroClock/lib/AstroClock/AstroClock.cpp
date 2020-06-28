#include <M5SysBase.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <Preferences.h>

#include "AstroClock.h"

extern  Preferences           preferences;
extern  M5SysBase             m5sys;



#define SCREEN_WIDTH          320
#define SCREEN_HEIGHT         240
#define SPRITE_WIDTH          233
#define SPRITE_HEIGHT         233
#define H_CENTER              116
#define V_CENTER              116
#define FULL_CIRCLE_RADIUS    108                           // leave room for appearance issues
#define SHORT_TICK_OFFSET     (FULL_CIRCLE_RADIUS - 18)
#define LONG_TICK_OFFSET      (FULL_CIRCLE_RADIUS / 3)
#define SPRITE_H_OFFSET       44                            // 44 is centered.
#define SPRITE_V_OFFSET       2
#define ADORNMENT_RADIUS      8

#define SUN_WEDGE_WIDTH       (SHORT_TICK_OFFSET - 12)
#define MOON_WEDGE_WIDTH      (SUN_WEDGE_WIDTH - 20)
#define SUN_COLOR             TFT_YELLOW
#define MOON_COLOR            TFT_CYAN
#define ASTRO_TWILIGHT_COLOR  M5.lcd.color565( 64,  64,  64)
#define NAUT_TWILIGHT_COLOR   M5.lcd.color565(112, 112, 112)
#define CIVIL_TWILIGHT_COLOR  M5.lcd.color565(192, 192, 192)

#define FRAME_OUTLINE_COLOR   WHITE
#define FRAME_RADIANT_COLOR   0x39E7                        // RGB565: 00111 001111 00111 = 25%:  0x39E7
#define FACE_NUMBER_COLOR     TFT_LIGHTGREY
#define LOCAL_TIME_COLOR      TFT_RED
#define SIDEREAL_TIME_COLOR   TFT_GREEN

#define TIME_URL              "http://192.168.50.10:8080/time"
#define DAY_URL               "http://192.168.50.10:8080/day"
#define UPDATED_PREFS_KEY     "updated"

#define DEG2RAD               0.0174532925
#define RAD2DEG               57.295779513



////////////////////////////////////////////////////////////////////////////////
//
// Constructor
//
AstroClock::AstroClock() : img("/ClockSprite.bmp") {
  updated = preferences.getInt(UPDATED_PREFS_KEY, -1);
  VERBOSE("AstroClock::AstroClock(): updated = %d\n", updated);
}


////////////////////////////////////////////////////////////////////////////////
//
//  Drawing routines
//
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
//
// Draw the local and sidereal time on the clockface.
//
void AstroClock::drawTime() {
  VERBOSE("AstroClock::drawTime()\n");
  double angle = secondsToAngle(T.local);
  drawClockHand(angle, LOCAL_TIME_COLOR, true);
  angle = secondsToAngle(T.lmst);
  drawClockHand(angle, SIDEREAL_TIME_COLOR, false);
}


////////////////////////////////////////////////////////////////////////////////
//
// from: https://github.com/Bodmer/TFT_eSPI/blob/master/examples/320%20x%20240/TFT_Pie_Chart/TFT_Pie_Chart.ino
// Draw circle segments
//
// x,y == coords of centre of circle
// start_angle = 0 - 360
// sub_angle   = 0 - 360 = subtended angle
// r = radius
// colour = 16 bit colour value
//
void AstroClock::fillSegment(int x, int y, int start_angle, int sub_angle, int r, unsigned int color) {
  VERBOSE("AstroClock::fillSegment(x=%d, y=%d, start_angle=%d, sub_angle=%d, r=%d, color=%h)\n", x, y, start_angle, sub_angle, r, color);
  // Calculate first pair of coordinates for segment start
  float sx = cos((start_angle - 90) * DEG2RAD);
  float sy = sin((start_angle - 90) * DEG2RAD);
  uint16_t x1 = sx * r + x;
  uint16_t y1 = sy * r + y;

  // Draw colour blocks every inc degrees
  for (int i = start_angle; i < start_angle + sub_angle; i++) {
    // Calculate pair of coordinates for segment end
    int x2 = cos((i + 1 - 90) * DEG2RAD) * r + x;
    int y2 = sin((i + 1 - 90) * DEG2RAD) * r + y;
    img.fillTriangle(x1, y1, x2, y2, x, y, color);
    // Copy segment end to segment start for next segment
    x1 = x2;
    y1 = y2;
  }
}


////////////////////////////////////////////////////////////////////////////////
//
// Draw a line from the center (skipping offset pixels) to the edge of the circle.
// Offset the angle so that 12 indicates straight up.
// Angle is expressed in radians, offset is # pixels
// from center to begin drawing.
// Draw to the Sprite "img"
//
void AstroClock::lineToEdge(double angle, int offset, uint32_t color) {
  int start_x, start_y, end_x, end_y;

  VERBOSE("AstroClock::lineToEdge(angle=%d/1000, offset=%d, color=%d\n", (int)(angle*1000), offset, color);
  angle   -= ((double)PI/2.0);
  start_x  = H_CENTER + offset * cos(angle);
  start_y  = V_CENTER + offset * sin(angle);
  end_x    = H_CENTER + FULL_CIRCLE_RADIUS * cos(angle);
  end_y    = V_CENTER + FULL_CIRCLE_RADIUS * sin(angle);
  img.drawLine(start_x, start_y, end_x, end_y, color);
  VERBOSE("x0=%d, y0=%d, x1=%d, y1=%d, color=%d\n", start_x, start_y, end_x, end_y, color);
}


////////////////////////////////////////////////////////////////////////////////
//
// Draw a triangle, with apex at center and spread over 1 degree at edge.
// Draw to the screen rather than to the sprite.
//
void AstroClock::drawClockHand(double angle, uint32_t color, bool adorn) {
  int x, y, x1, y1, x2, y2;
  const double diff = (double)PI / 500.0;

  VERBOSE("AstroClock::drawClockHand(angle=%d/1000, color=%d, adorn=%d\n", (int)(angle*1000), color, adorn);
  angle -= ((double)PI/2.0);
  x     = H_CENTER + SPRITE_H_OFFSET;
  y     = V_CENTER + SPRITE_V_OFFSET;
  x1    = H_CENTER + SPRITE_H_OFFSET + FULL_CIRCLE_RADIUS * cos(angle - diff);
  y1    = V_CENTER + SPRITE_V_OFFSET + FULL_CIRCLE_RADIUS * sin(angle - diff);
  x2    = H_CENTER + SPRITE_H_OFFSET + FULL_CIRCLE_RADIUS * cos(angle + diff);
  y2    = V_CENTER + SPRITE_V_OFFSET + FULL_CIRCLE_RADIUS * sin(angle + diff);
  M5.Lcd.fillTriangle(x, y, x1, y1, x2, y2, color);
  if(adorn) {
    x   = H_CENTER + SPRITE_H_OFFSET + FULL_CIRCLE_RADIUS * cos(angle);
    y   = V_CENTER + SPRITE_V_OFFSET + FULL_CIRCLE_RADIUS * sin(angle);
    M5.Lcd.drawCircle(x, y, ADORNMENT_RADIUS,   color);
    M5.Lcd.drawCircle(x, y, ADORNMENT_RADIUS-1, color);
  }
}


////////////////////////////////////////////////////////////////////////////////
//
// Draw the areas for moon, sun, and twilights, using D data.
// Note that D changes only once per day.
// Twilight extents are exaggerated on the inner side becuse rounding errors produce gaps otherwise.

void AstroClock::drawWedges() {
  VERBOSE("AstroClock::drawWedges()");
  fillSegment(H_CENTER, V_CENTER, D.bmatAngle,     D.bmntAngle - D.bmatAngle + 1, SUN_WEDGE_WIDTH,  ASTRO_TWILIGHT_COLOR);
  fillSegment(H_CENTER, V_CENTER, D.bmntAngle,     D.bmctAngle - D.bmntAngle + 1, SUN_WEDGE_WIDTH,  NAUT_TWILIGHT_COLOR);
  fillSegment(H_CENTER, V_CENTER, D.bmctAngle,     D.srAngle   - D.bmctAngle + 1, SUN_WEDGE_WIDTH,  CIVIL_TWILIGHT_COLOR);
  fillSegment(H_CENTER, V_CENTER, D.ssAngle - 1,   D.eectAngle - D.ssAngle + 1,   SUN_WEDGE_WIDTH,  CIVIL_TWILIGHT_COLOR);
  fillSegment(H_CENTER, V_CENTER, D.eectAngle - 1, D.eentAngle - D.eectAngle + 1, SUN_WEDGE_WIDTH,  NAUT_TWILIGHT_COLOR);
  fillSegment(H_CENTER, V_CENTER, D.eentAngle - 1, D.eeatAngle - D.eentAngle + 1, SUN_WEDGE_WIDTH,  ASTRO_TWILIGHT_COLOR);
  fillSegment(H_CENTER, V_CENTER, D.srAngle,       D.ssAngle   - D.srAngle,       SUN_WEDGE_WIDTH,  SUN_COLOR);
  int lunarExtent = (D.msAngle < D.mrAngle) ? D.msAngle + 360 - D.mrAngle : D.msAngle - D.mrAngle;
  fillSegment(H_CENTER, V_CENTER, D.mrAngle, lunarExtent, MOON_WEDGE_WIDTH, MOON_COLOR);
}


////////////////////////////////////////////////////////////////////////////////
//
// Draw the circle, lines and numbers of the clock face.
//
void AstroClock::drawClockFace() {
  VERBOSE("AstroClock::drawClockFace()");
  for(int i = 0; i < 96; i++) {
    lineToEdge(PI/48.0*i, SHORT_TICK_OFFSET, FRAME_RADIANT_COLOR);    
  }
  for(int i = 0; i < 24; i++) {
    lineToEdge(PI/12.0*i, LONG_TICK_OFFSET, FRAME_RADIANT_COLOR);    
  }
  for(int i = 0; i < 4; i++) {
    lineToEdge(PI/2.0*i, 0, FRAME_RADIANT_COLOR);    
  }
  img.setTextColor(FACE_NUMBER_COLOR);
  img.drawNumber(12, H_CENTER - 8, V_CENTER - FULL_CIRCLE_RADIUS + 4, 2);
  img.drawNumber(18, H_CENTER + FULL_CIRCLE_RADIUS - 24, V_CENTER - 10, 2);
  img.drawNumber(0, H_CENTER - 3, V_CENTER + FULL_CIRCLE_RADIUS - 20, 2);
  img.drawNumber(6, H_CENTER - FULL_CIRCLE_RADIUS + 10, V_CENTER - 7, 2);
  img.drawCircle(H_CENTER, V_CENTER, FULL_CIRCLE_RADIUS, FRAME_OUTLINE_COLOR);
}


////////////////////////////////////////////////////////////////////////////////
//
// Get Time and Day data, else get Time and see if midnight has passed; if so,
// refresh Day data, then erase screen, draw sun and moon wedges, clock face,
// and time indicators.
//
void AstroClock::draw() {
  VERBOSE("AstroClock::draw()\n");
  
  bool newDay = false;
  if(-1 == T.utc) {
    fetchTime();
    fetchDay();
    M5.Lcd.fillScreen(TFT_BLACK); // Screen could still have connection info
    if(updated != T.doy) {
      newDay = true;
    }
  }
  else {
    fetchTime();
    VERBOSE("Updated = %d, T.doy = %d\n", updated, T.doy);
    if(updated != T.doy) {
      fetchDay(); // The day has changed, relead Day info
      newDay = true;
    }
  }
  // We only need to redraw the sprite once a day, or when the sprite is new.
  if(newDay) {
    img.createSprite(SPRITE_WIDTH, SPRITE_HEIGHT);
    img.fillSprite(TFT_BLACK);
    drawWedges();
    drawClockFace();
    INFO("Redrew clock face\n");
    img.pushSprite(SPRITE_H_OFFSET, SPRITE_V_OFFSET);
    preferences.putInt(UPDATED_PREFS_KEY,-1);     // In case image save is incomplete
    updated = -1;
    img.saveToBmp();
    preferences.putInt(UPDATED_PREFS_KEY, T.doy); // Save Day of Year to preferences, so we''l know it even after a reset
    updated = T.doy;                              // Remember locally so we don't have to reload prefs constantly
    img.deleteSprite();
    INFO("Wrote Clock Sprite to SPIFFS, updated to %d\n", updated);
  }
  else {
    VERBOSE("Loaded Clock Sprite image from SPIFFS.\n");
    M5.Lcd.drawBmpFile(SPIFFS, img.name(), SPRITE_H_OFFSET, SPRITE_V_OFFSET);
  }
  drawTime();
}


////////////////////////////////////////////////////////////////////////////////
//
//  Networking and Configuration
//
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
//
// Contact the Astro server to get the Time info.
// Gets a response like:
// {"utc": "05:27:06", "local": "21:27:06", "solar": "21:07:25", "timezone": "America/Los_Angeles",
// "jdate": 2458869.7271624305, "gmst": "13:27:20", "lmst": "05:18:36", "doy": 20}
// JSON_OBJECT_SIZE(8) + 256
// Return true and update T if successful, else return false and do not change T.
//
bool AstroClock::fetchTime() {
  VERBOSE("AstroClock::fetchTime(): URL=%s\n", TIME_URL);
  HTTPClient http;
  http.begin(TIME_URL);
  int httpCode = http.GET();
  INFO("AstroClock::fetchTime httpCode = %d\n", httpCode);
  if(httpCode == HTTP_CODE_OK) {
    Stream& response = http.getStream();
    DynamicJsonDocument doc(JSON_OBJECT_SIZE(8) + 256);
    DeserializationError error = deserializeJson(doc, response);
    if(error) {
      ERROR("AstroClock::fetchTime: DeserializationError = %s\n", error.c_str());
      VERBOSE("> return false\n");
      return false;
    }
    T.utc         = timeStringToSeconds(doc["utc"].as<char *>());
    T.local       = timeStringToSeconds(doc["local"].as<char *>());
    T.solar       = timeStringToSeconds(doc["solar"].as<char *>());
    T.jdate       = doc["jdate"].as<double>();
    T.gmst        = timeStringToSeconds(doc["gmst"].as<char *>());
    T.lmst        = timeStringToSeconds(doc["lmst"].as<char *>());
    T.doy         = doc["doy"].as<int>();
    T.acquisition = millis();
    strlcpy(T.timezone, doc["timezone"].as<char *>(), TIMEZONE_STRLEN-1);
    VERBOSE("> return true\n");
    return true;
  }
  VERBOSE("> return false\n");
  return false;
}


////////////////////////////////////////////////////////////////////////////////
//
// Contact the Astro server to get the Day info (only changes daily.)
// Gets a response like:
// {"DATE": "2020-01-22", "LAT": 47.7254, "LON": -122.18024, "ALT": 77.9, "BMAT": "05:56:38",
// "BMNT": "06:33:12", "BMCT": "07:10:58", "SRISE": "07:47:16", "SCUL": "12:20:28",
// "SCALT": 22.62, "SSET": "16:53:41", "EECT": "17:30:00", "EENT": "18:07:47", "EEAT": "18:44:23",
// "LPHA": 4.2, "MRISE": "06:17:16", "LCUL": "10:33:49", "LCALT": 18.25, "MSET": "14:50:19"}
// JSON_OBJECT_SIZE(19) + 256
// Return true and update D if successful, else return false and do not change D.
//
bool AstroClock::fetchDay() {
  VERBOSE("AstroClock::fetchDay(): URL=%s\n", DAY_URL);
  HTTPClient http;
  http.begin(DAY_URL);
  int httpCode = http.GET();
  INFO("AstroClock::fetchDay httpCode = %d\n", httpCode);
  if(httpCode == HTTP_CODE_OK) {
    Stream& response = http.getStream();
    DynamicJsonDocument doc(JSON_OBJECT_SIZE(19) + 256);
    DeserializationError error = deserializeJson(doc, response);
    if(error) {
      ERROR("AstroClock::fetchDay: DeserializationError = %s\n", error.c_str());
      VERBOSE("> return false\n");
      return false;
    }
    const char* bmat  = doc["BMAT"].as<char *>();
    const char* bmnt  = doc["BMNT"].as<char *>();
    const char* bmct  = doc["BMCT"].as<char *>();
    const char* srise = doc["SRISE"].as<char *>();
    const char* sset  = doc["SSET"].as<char *>();
    const char* mrise = doc["MRISE"].as<char *>();
    const char* mset  = doc["MSET"].as<char *>();
    const char* eect  = doc["EECT"].as<char *>();
    const char* eent  = doc["EENT"].as<char *>();
    const char* eeat  = doc["EEAT"].as<char *>();
    
    D.bmatAngle       = timeStringToAngle(bmat)  * RAD2DEG;
    D.bmntAngle       = timeStringToAngle(bmnt)  * RAD2DEG;
    D.bmctAngle       = timeStringToAngle(bmct)  * RAD2DEG;
    D.srAngle         = timeStringToAngle(srise) * RAD2DEG;
    D.ssAngle         = timeStringToAngle(sset)  * RAD2DEG;
    D.mrAngle         = timeStringToAngle(mrise) * RAD2DEG;
    D.msAngle         = timeStringToAngle(mset)  * RAD2DEG;
    D.eectAngle       = timeStringToAngle(eect)  * RAD2DEG;
    D.eentAngle       = timeStringToAngle(eent)  * RAD2DEG;
    D.eeatAngle       = timeStringToAngle(eeat)  * RAD2DEG;
    VERBOSE("> return true\n");
    return true;
  }
  VERBOSE("> return false\n");
  return false;
}


////////////////////////////////////////////////////////////////////////////////
//
//  Utility functions
//
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
//
// Take a short time string (like 22:38:25) and change it into an angle
// in radians representing the portion of a 24 hour day.
// Rotate by 180 degrees to match clock face.
// TODO: let's remove the preemptory corrections.
//
int AstroClock::timeStringToSeconds(const char* tstring) {
  int hour, minute, second;
  
  VERBOSE("AstroClock::timeStringToSeconds: %s\n", tstring ? tstring : "nullptr");
  if(nullptr == tstring) return 0;
  sscanf(tstring, "%2d:%2d:%2d", &hour, &minute, &second);
  VERBOSE("Hour: %d, Minute: %d, Second: %d\n", hour, minute, second);
  int seconds = second + (minute * 60) + (hour * 3600);
  VERBOSE("> return: Seconds = %d\n", seconds);
  return seconds;
}


////////////////////////////////////////////////////////////////////////////////
//
// Take a short time string (like 22:38:25) and change it into an angle
// in radians representing the portion of a 24 hour day.
// Rotate by 180 degrees to match clock face.
// TODO: let's remove the preemptory corrections.
//
double AstroClock::timeStringToAngle(const char* tstring) {
  int hour,  minute, second;
    
  VERBOSE("AstroClock::timeStringToAngle: %s\n", tstring ? tstring : "nullptr");
  if(nullptr == tstring) return 0.0;
  sscanf(tstring, "%2d:%2d:%2d", &hour, &minute, &second);
  VERBOSE("Hour: %d, Minute: %d, Second: %d\n", hour, minute, second);
  int t_total = second + (minute * 60) + (hour * 3600);
  VERBOSE("Total= %d\n", t_total);
  double angle = (double)t_total / (double)86400.0 * double(PI * 2.0);
  VERBOSE("> return: angle * 1000 = %d\n", (int)(angle*1000));
  return angle + (double)PI;
}


////////////////////////////////////////////////////////////////////////////////
//
// Change the number of seconds since midnight to an angle corrected for display on clock face.
// TODO: let's remove the preemptory corrections.
//
double AstroClock::secondsToAngle(int seconds) {
  VERBOSE("AstroClock::secondsToAngle(seconds=%d)\n", seconds);
  double angle = (double)seconds / (double)86400.0 * double(PI * 2.0);
  return angle + (double)PI;
}
