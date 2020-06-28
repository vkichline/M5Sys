#include <ArduinoJson.h>
#include <math.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Preferences.h>
#include <FS.h>
#include <SPIFFS.h>

#include <M5SysBase.h>
#include <AstroClock.h>
#include <Planets.h>


#define SCREEN_WIDTH            320
#define SCREEN_HEIGHT           240
#define FORMAT_SPIFFS_IF_FAILED true
#define SSID                    String("AstroPi")
#define NETWORK_NAME            "AstroPi"


enum  Position                  { top_left, top_right, bottom_left, bottom_right };

const char* zodiac[]          = { "    Pcs", "    Ari", "    Tau", "    Gem", "    Can", "    Leo",
                                  "    Vir", "    Lib", "    Sco", "    Sgr", "    Cap", "    Aqr" };

int         loopCount         = 0;
int         displayLocalTime  = 0;
int         displaySidTime    = 0;

// wifi config store
M5SysBase   m5sys;
Preferences preferences;
String      wifi_ssid;
String      wifi_password;

AstroClock* clockface;
Planets*    planets;


////////////////////////////////////////////////////////////////////////////////
//
//  Networking and Configuration
//
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
//
// Wait up to 15 seconds to connect, with feedback.
//
boolean checkConnection() {
  VERBOSE("checkConnection()\n");
  int count = 0;
  M5.Lcd.setTextFont(2);
  INFO("Waiting for Wi-Fi connection.\n");
  M5.Lcd.print("Waiting for Wi-Fi connection");
  while(count < 30) {
    if (WiFi.status() == WL_CONNECTED) {
      INFO("Connected.\n");
      M5.Lcd.println("\nConnected.");
      VERBOSE("> return true\n");
      return (true);
    }
    delay(500);
    M5.Lcd.print(".");
    count++;
  }
  ERROR("Connection timed out.\n");
  M5.Lcd.println("\nTimed out.");
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
// Print a string in the given corner of the screen
//
void displayInCorner(Position pos, const char* buffer) {
  VERBOSE("displayInCorner(pos=%d, buffer=%s)\n", pos, buffer);
  int width, x, y;
  
  width = M5.Lcd.textWidth(buffer, 4);
  switch(pos) {
    case top_left:
      x = 0;
      y = 0;
      break;
    case top_right:
      x = SCREEN_WIDTH - width;
      y = 0;
      break;
    case bottom_left:
      x = 0;
      y = SCREEN_HEIGHT - M5.Lcd.fontHeight(4);
      break;
    case bottom_right:
      x = SCREEN_WIDTH - width;
      y = SCREEN_HEIGHT - M5.Lcd.fontHeight(4);
      break;
    default:
      ERROR("displayInCorner: Unknown Position: %d.", pos);
      return;
  }
  M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Lcd.setCursor(x, y, 4);
  M5.Lcd.print(buffer);
}


////////////////////////////////////////////////////////////////////////////////
//
// t is in seconds; displays time on the left of display, at given position.
//
void displayNumericTime(int t, Position pos) {
  VERBOSE("displayNumericTime(t=%d, pos=%d)\n", t, pos);
  char buffer[16];
  int second, minute, hour;
  
  second  = t % 60;
  t /= 60;
  minute  = t % 60;
  t /= 60;
  hour    = t;
  sprintf(buffer, "%d:%02d:%02d", hour, minute, second);
  displayInCorner(pos, buffer);
}


////////////////////////////////////////////////////////////////////////////////
//
//  Arduino Program Control
//
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
//
// Start the file system and list the names and sizes of all files.
//
bool startSpiffs() {
  VERBOSE("startSpiffs\n");
  if(!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED)){
    ERROR("SPIFFS Mount Failed.\n");
    return false;
  }
  File root = SPIFFS.open("/");
  File file = root.openNextFile();
  while(file){
    DEBUG("  File: %s (%d bytes)\n", file.name(), file.size());
    file = root.openNextFile();
  }
  return true;
}


////////////////////////////////////////////////////////////////////////////////
//
// Standard Arduino startup routine.
// Connect to wifi and draw the clock face.
//
void setup() {
  m5sys.begin("AstroClock", NETWORK_NAME);
  M5.Lcd.setRotation(1);
  clockface = new AstroClock();
  planets   = new Planets();
  startSpiffs();
  checkConnection();
}


////////////////////////////////////////////////////////////////////////////////
//
// Main loop. Display time once a second, update planets and clockface once a minute.
// On button presses, select a visible planet.
//
void loop() {
  int   originalPlanet = planets->currentPlanet;
  
  M5.update();
  if(M5.BtnA.wasReleased()) planets->inc();
  if(M5.BtnC.wasReleased()) planets->dec();

  if(planets->anyPlanetSelected()) {
    // Once a minute update the planet location data
    if(0 == loopCount % 600) {
      planets->update();
      // If the current planet is no longer visible, exit and re-compute.
      if(!planets->planets[planets->currentPlanet].visible) {
        planets->inc();
        return;
      }
      planets->drawAltAzm();
    }
    else if(originalPlanet != planets->currentPlanet) {
      planets->drawAltAzm();
    }
    // Once a second update the time display
    if((0 == loopCount % 10) || (originalPlanet != planets->currentPlanet)) {
      displayNumericTime(displayLocalTime++, top_left);
      displayNumericTime(displaySidTime++, top_right);
    }
  }
  else {
    // Once a minute update the time and planets
    if((0 == loopCount % 600) || (originalPlanet != planets->currentPlanet)) {
      planets->update();
      clockface->draw();
      displayLocalTime  = clockface->getLocalTime();
      displaySidTime    = clockface->getSiderealTime();
    }
    // Once a second update the time display
    if((0 == loopCount % 10)  || (originalPlanet != planets->currentPlanet)) {
      displayNumericTime(displayLocalTime++, top_left);
      displayNumericTime(displaySidTime, top_right);
      displayInCorner(bottom_right, zodiac[(int)(displaySidTime++ / 7200)]);
    }
  }
  planets->drawMenu();

  loopCount++;
  //long tDelay = (millis() - clockface->T.acquisition);
  //ERROR("Delay = %d, Loop = %d\n", tDelay, loopCount);
  delay(93);  // Remainder of a tenth of a second (on M5Stack Gray)
}
