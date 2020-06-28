#include <M5SysBase.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include "Planets.h"

#define SCREEN_WIDTH          320
#define SCREEN_HEIGHT         240
#define SPRITE_WIDTH          233
#define SPRITE_HEIGHT         233
#define H_CENTER              116
#define V_CENTER              116
#define SPRITE_H_OFFSET       44                          // 44 is centered.
#define SPRITE_V_OFFSET       2
#define FULL_CIRCLE_RADIUS    108                         // leave room for appearance issues
#define SHORT_TICK_OFFSET     (FULL_CIRCLE_RADIUS - 8)
#define MEDIUM_TICK_OFFSET    (FULL_CIRCLE_RADIUS - 24)
#define LONG_TICK_OFFSET      (FULL_CIRCLE_RADIUS / 2)

#define COMPASS_COLOR         0x39E7                      // RGB565: 00111 001111 00111 = 25%:  0x39E7
#define DIRECTION_COLOR       TFT_LIGHTGREY
#define CPMPASS_CIRCLE_COLOR  TFT_WHITE
#define AZIMUTH_COLOR         TFT_RED

#define DEG2RAD               0.0174532925
#define RAD2DEG               57.295779513

extern  M5SysBase             m5sys;


////////////////////////////////////////////////////////////////////////////////
//
// Constructor
//
Planets::Planets() : img("/CompassSprite.bmp")  {
  VERBOSE("Planets::Planets()\n");
  currentPlanet = -1;     // -1 means no selection
}


////////////////////////////////////////////////////////////////////////////////
//
// Increment the currentPlanet. If > max, go to -1 so no planet is selected.
//
void Planets::inc() {
  VERBOSE("Planets::inc()\n");
  if(anyPlanetsVisible()) {
    // currentPlanet can be -1 for no selection, or 0 - MAX_PLANET (or NUM_PLANETS - 1)
    int startPlanet = currentPlanet;
    // Decrement. We've established that there's at least one visible planet
    while(true) {
      if(-1 == currentPlanet) currentPlanet = MAX_PLANET + 1;
      currentPlanet--;
      if(0 > currentPlanet) break;
      if(planets[currentPlanet].visible) {
        if(currentPlanet == startPlanet) currentPlanet = -1;
        break;
      }
    }
  }
  else {
    currentPlanet = -1;
  }
}


////////////////////////////////////////////////////////////////////////////////
//
// Decrement the currentPlanet. If zero, go to -1 so no planet is selected.
//
void Planets::dec() {
  VERBOSE("Planets::dec()\n");
  if(anyPlanetsVisible()) {
    // currentPlanet can be -1 for no selection, or 0 - MAX_PLANET (or NUM_PLANETS - 1)
    int startPlanet = currentPlanet;
    // Decrement. We've established that there's at least one visible planet
    while(true) {
      currentPlanet++;
      if(MAX_PLANET < currentPlanet) {
        currentPlanet = -1;
        break;
      }
      if(planets[currentPlanet].visible) {
        if(currentPlanet == startPlanet) currentPlanet = -1;
        break;
      }
    }
  }
  else {
    currentPlanet = -1;
  }
}


////////////////////////////////////////////////////////////////////////////////
//
// Return true if any planets are known to be visible (with current data.)
//
bool Planets::anyPlanetsVisible() {
  VERBOSE("Planets::anyPlanetsVisible()\n");
  for(int i = 0; i < NUM_PLANETS; i++) {
    if(planets[i].initialized && planets[i].visible) return true;
    VERBOSE("> return true\n");
  }
  VERBOSE("> return false\n");
  return false;
}


////////////////////////////////////////////////////////////////////////////////
//
// Return the altitude of the current planet in degrees (or 0)
//
float Planets::getAlt() {
  VERBOSE("Planets::getAlt()\n");
  if(-1 != currentPlanet) {
    VERBOSE("> return %d\n", planets[currentPlanet].alt);
    return planets[currentPlanet].alt;
  }
  VERBOSE("> return 0.0");
  return 0.0;
}


////////////////////////////////////////////////////////////////////////////////
//
// Return the azimuth of the current planet in degrees (or 0)
//
float Planets::getAzm() {
  VERBOSE("Planets::getAzm()\n");
  if(-1 != currentPlanet) {
    VERBOSE("> return %d\n", planets[currentPlanet].azm);
    return planets[currentPlanet].azm;
  }
  VERBOSE("> return 0.0");
  return 0.0;
}



////////////////////////////////////////////////////////////////////////////////
//
//  Networking and Configuration
//
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
//
// Contact the Astro server to get the WhatsUp info.
//
bool Planets::update() {
  VERBOSE("Planets::update(): url=%s\n", WHATSUP_URL);
  HTTPClient http;
  http.begin(WHATSUP_URL);
  int httpCode = http.GET();
  INFO("Planets::fetchPlanets httpCode = %d\n", httpCode);
  if(httpCode == HTTP_CODE_OK) {
    Stream& response = http.getStream();
    DynamicJsonDocument doc(JSON_OBJECT_SIZE(7) + 7 * JSON_OBJECT_SIZE(4) + 1024);
    DeserializationError error = deserializeJson(doc, response);
    if(error) {
      ERROR("Planets::fetchPlanets: DeserializationError = %s\n", error.c_str());
      VERBOSE("> return false\n");
      return false;
    }
    for(int i = 0; i < NUM_PLANETS; i++) {
      planets[i].initialized  = true;
      JsonObject planet       = doc[planets[i].name].as<JsonObject>();
      if(planet) {
        planets[i].visible    = true;
        planets[i].alt        = planet["alt"];
        planets[i].azm        = planet["azm"];
        planets[i].distance   = planet["distance"];
        planets[i].illum      = planet["illum"];
      }
      else {
        planets[i].visible    = false;
      }
    }
    VERBOSE("> return true\n");
    return true;
  }
  VERBOSE("> return false\n");
  return false;
}


////////////////////////////////////////////////////////////////////////////////
//
//  Drawing routines
//
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
//
// show abbreviations for each planet down the left side.
// highlight appropriately for uninited, visible, or non-visible.
//
void Planets::drawMenu() {
  //  VERBOSE("Planets::drawMenu()\n"); This is called too often to print every time
  uint16_t  color = TFT_BLACK;
  
  M5.Lcd.setCursor(0, 54, 4);
  for(int i = 0; i < NUM_PLANETS; i++) {
    if(!planets[i].initialized) {
      color = UNINITED_MENU_COLOR;
    }
    else {
      if(i == currentPlanet) {
        color = SELECTED_MENU_COLOR;
      }
      else {
        if(planets[i].visible) {
          color = ENABLED_MENU_COLOR;
        }
        else {
          color = DISABLED_MENU_COLOR;
        }
      }
    }
    M5.Lcd.setTextColor(color);
    M5.Lcd.println(planets[i].abbreviation);
  }
}


////////////////////////////////////////////////////////////////////////////////
//
// Draw a line from the center (skipping offset pixels) to the edge of the circle.
// Offset the angle so that 12 indicates straight up.
// Angle is expressed in radians, offset is # pixels
// from center to begin drawing.
//
void Planets::lineToEdge(double angle, int offset, uint32_t color, TFT_eSPI& view) {
  int start_x, start_y, end_x, end_y;

  VERBOSE("Planets::lineToEdge(angle=%d/1000, offset=%d, color=%d\n", (int)(angle*1000), offset, color);
  angle      -= ((double)PI/2.0);
  start_x     = H_CENTER + offset * cos(angle);
  start_y     = V_CENTER + offset * sin(angle);
  end_x       = H_CENTER + FULL_CIRCLE_RADIUS * cos(angle);
  end_y       = V_CENTER + FULL_CIRCLE_RADIUS * sin(angle);
  if(&M5.Lcd == &view) {
    start_x  += SPRITE_H_OFFSET;
    start_y  += SPRITE_V_OFFSET;
    end_x    += SPRITE_H_OFFSET;
    end_y    += SPRITE_V_OFFSET;
  }
  view.drawLine(start_x, start_y, end_x, end_y, color);
  VERBOSE("x0=%d, y0=%d, x1=%d, y1=%d, color=%d\n", start_x, start_y, end_x, end_y, color);
}


////////////////////////////////////////////////////////////////////////////////
//
// Draw the compass with altitude and azimuth of currentPlanet
//
void Planets::drawAltAzm() {
  VERBOSE("Planets::drawAltAzm(). currentPlanet = %d\n", currentPlanet);
  if(-1 != currentPlanet) {
    if(SPIFFS.exists(img.name())) {
      M5.Lcd.drawBmpFile(SPIFFS, img.name(), SPRITE_H_OFFSET, SPRITE_V_OFFSET);
    }
    else {
      img.createSprite(SPRITE_WIDTH, SPRITE_HEIGHT);
      img.fillScreen(TFT_BLACK);
      for(int i = 0; i < 128; i++) {
        lineToEdge(PI/64.0*i, SHORT_TICK_OFFSET, COMPASS_COLOR, img);    
      }
      for(int i = 0; i < 32; i++) {
        lineToEdge(PI/16.0*i, MEDIUM_TICK_OFFSET, COMPASS_COLOR, img);    
      }
      for(int i = 0; i < 8; i++) {
        lineToEdge(PI/4.0*i, LONG_TICK_OFFSET, COMPASS_COLOR, img);    
      }
      for(int i = 0; i < 4; i++) {
        lineToEdge(PI/2.0*i, 0, COMPASS_COLOR, img);    
      }
      img.setTextColor(DIRECTION_COLOR, TFT_BLACK);
      img.drawString("N", H_CENTER - 8, V_CENTER - FULL_CIRCLE_RADIUS + 10, 4);
      img.drawString("E", H_CENTER + FULL_CIRCLE_RADIUS - 28, V_CENTER - 10, 4);
      img.drawString("S", H_CENTER - 8, V_CENTER + FULL_CIRCLE_RADIUS - 28, 4);
      img.drawString("W", H_CENTER - FULL_CIRCLE_RADIUS + 12, V_CENTER - 10, 4);
      img.drawCircle(H_CENTER, V_CENTER, FULL_CIRCLE_RADIUS, CPMPASS_CIRCLE_COLOR);
      img.pushSprite(SPRITE_H_OFFSET, SPRITE_V_OFFSET);
      img.saveToBmp();
      img.deleteSprite();
    }
    drawAzm(planets[currentPlanet].azm);
    drawAlt(planets[currentPlanet].alt);
  }
}


////////////////////////////////////////////////////////////////////////////////
//
// Draw a line for the azimuth and print the value above or below the centerline.
//
void Planets::drawAzm(float azm) {
  int x, y, width;
  char  buffer[16];

  VERBOSE("Planets::drawAzm(azm=%d/1000\n", (int)(azm * 1000));
  if(-1 != currentPlanet) {
    double azm = planets[currentPlanet].azm;
    lineToEdge(azm * DEG2RAD, 0, AZIMUTH_COLOR);

    if(azm > 90.0 && azm < 270.0) {
      y = V_CENTER - 32;
    }
    else {
      y = V_CENTER + 12;
    }
    sprintf(buffer, "%.2f", azm);
    width = M5.Lcd.textWidth(buffer, 4);
    x = H_CENTER + SPRITE_H_OFFSET - (width / 2);
    M5.Lcd.setCursor(x, y, 4);
    M5.Lcd.print(buffer);
  }
}


////////////////////////////////////////////////////////////////////////////////
//
// Draw altitude in the lower right corner.
//
void Planets::drawAlt(float alt) {
  VERBOSE("Planets::drawAlt(alt=%d/1000\n", (int)(alt * 1000));
  int   x, y, width;
  char  buffer[16];
  
  VERBOSE("\n");
  if(-1 != currentPlanet) {
    sprintf(buffer, "%.2f", planets[currentPlanet].alt);
    width = M5.Lcd.textWidth(buffer, 4);
    x = SCREEN_WIDTH - width;
    y = SCREEN_HEIGHT - M5.Lcd.fontHeight(4);
    M5.Lcd.setCursor(x, y, 4);
    M5.Lcd.setTextColor(CPMPASS_CIRCLE_COLOR, TFT_BLACK);
    M5.Lcd.print(buffer);
  }
}
