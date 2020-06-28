#include <Preferences.h>
#include <WiFi.h>
#include <M5StackUpdater.h>
#include "M5SysBase.h"

#define M5EZ_PREFS_NAME  "M5ez"


void M5SysBase::begin(const char* appName, const char* connection) {
  M5.begin();
  log.begin(appName);
  startup_test();     // If we're leaving the app, there's no reason to start wifi
  M5.Power.begin();   // handle battery charging
  start_wifi(connection);
}


// Start the SPIFFS file system and return the status
//
bool M5SysBase::start_spiffs(bool formatOnFailure) {
  if(SPIFFS.begin(false)) {
    log.debug("SPIFFS file system started\n");
    return true;
  }
  else {
    log.error("SPIFFS File System startup failed.\n");
    return false;
  }
}


// Test the buttons (at startup) and take appropriate action
//
void M5SysBase::startup_test() {
  log.verbose("M5SysBase::startup_test()\n");
  if(digitalRead(BUTTON_A_PIN) == 0) {
    log.info("M5SysBase::startup_test(): Caught button A\n");
    goHome();
  }
  // if(digitalRead(BUTTON_C_PIN) == 0) {
  //   Serial.println("Loading Settings...");
  //   updateFromFS(SD, "/M5Settings.bin");
  //   ESP.restart();
  // }
}


// Launch menu.bin (the M5Home program)
//
void M5SysBase::goHome() {
  log.info("Loading M5Sys Home...\n");
  updateFromFS(SD);
  ESP.restart();
}


// Launch a program from SD by the name given.
// Canonic name must begine ith '/' and end with '.bin',
// but these are provided if omitted.
//
void M5SysBase::launch(const char* progName) {
  log.info("M5SysBase::launch(%s)\n", progName);
  static String whack = String("/");
  static String bin   = String(".bin");
  String target(progName);

  target.trim();
  if(!target.startsWith(whack)) target = whack + target;
  if(!target.endsWith(bin))     target = target + bin;
  updateFromFS(SD, target);
  ESP.restart();
}


// Idle while waiting for any of the three M5 buttons to be pressed.
// If wait_for_clear is true, wait until all buttons are up before testing.
// (Use this if a previous button press might not yet have been fully released.)
//
void M5SysBase::wait_for_any_button(bool wait_for_clear) {
  log.verbose("M5SysBase::wait_for_any_button(%s)\n", wait_for_clear ? "true" : "false");
  if(wait_for_clear) {
    log.debug("waiting for all buttons to be up\n");
    while(true) {
      M5.update();
      if(!M5.BtnA.isPressed() && !M5.BtnB.isPressed() && !M5.BtnC.isPressed()) break;
    delay(10);
    }
    log.debug("done waiting\n");
  }
  while(true) {
    M5.update();
    if(M5.BtnA.isPressed() || M5.BtnB.isPressed() || M5.BtnC.isPressed()) break;
    delay(10);
  }
}


////////////////////////////////////////////////////////////////////////////////
//
//  WiFi System
//
////////////////////////////////////////////////////////////////////////////////

// The network connection can be specified as:
//  NETWORK_CONNECTION_NONE No network connection will be made
//  NETWORK_CONNECTION_AUTO Autoconnect will select the strongest known connection
//  The name of any defined M5ez AutoConnect network
//
void M5SysBase::start_wifi(const char* connection) {
  log.verbose("start_wifi(%s)\n", connection);
  WiFi.setAutoConnect(false);
  if(0 == strcmp(NETWORK_CONNECTION_NONE, connection)) {
    disconnect();
  }
  else if(0 == strcmp(NETWORK_CONNECTION_AUTO, connection)) {
    auto_connect();
  }
  else {
    connect_to_ssid(connection);
  }
}


// Connection was set to None.  Disconnect from the network.
//
void M5SysBase::disconnect() {
  log.verbose("M5SysBase::disconnect()\n");
  log.info("setting network connection to None\n");
  WiFi.disconnect();
  WiFi.setAutoConnect(false);
}


// Relay on the esp32/arduino autoconnect function
//
void M5SysBase::auto_connect() {
  log.verbose("M5SysBase::auto_connect()\n");
  log.info("setting network connection to Autoconnect\n");
  WiFi.disconnect();
  WiFi.setAutoConnect(true);
}


// Look the given ssid up in the M5ez preferences. If found, extract
// the matching password and connect. If not found, return false.
// Return true if WiFi.begin is called. Check connection asynchronously.
//
bool M5SysBase::connect_to_ssid(const char* connection) {
  log.verbose("M5SysBase::connect_to_ssid(%s)\n", connection);
  Preferences preferences;
  String      idx;
  String      password;
  uint8_t     index = 1;

  preferences.begin(M5EZ_PREFS_NAME, true);  // read-only
  while(true) {
    idx         = "SSID" + (String)index;
    String ssid = preferences.getString(idx.c_str(), "");
    if(ssid.length() == 0) {
      log.error("Network %s not found in preferences. Disabling WiFi.\n", connection);
      WiFi.disconnect();
      WiFi.setAutoConnect(false);
      return false;
    }
    if(0 == ssid.compareTo(connection)) {
      idx = "key" + (String)index;
      password = preferences.getString(idx.c_str(), "");
      break;
    }
    index++;
  }
  preferences.end();

  log.info("Connecting to network %s\n", connection);
  WiFi.begin(connection, password.c_str());
  return true;
}
