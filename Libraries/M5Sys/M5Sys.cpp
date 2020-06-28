#include <Preferences.h>
#include "M5Sys.h"

#define _M5SYSBUFFERSIZE  64
#define M5SYS_PREFS_NAME  "M5Sys"
#define CONNECTION_TYPE   "conn_type"


// This will start M5Stack, etc.
// The name of the app will be logged at INFO level
// The connection can be one of:
// The network connection can be specified as:
//  NETWORK_CONNECTION_NONE No network connection will be made
//  NETWORK_CONNECTION_AUTO Autoconnect will select the strongest known connection
//  NETWORK_CONNECTION_UI   M5ez UI will list all favorites and will allow the user to pick one
//  The name of any defined M5ez AutoConnect network
//
void M5Sys::begin(const char* appName, const char* connection) {
  #include <themes/default.h>
  #include <themes/dark.h>
  ez.begin();
  log.begin(appName);
  position.begin(&log);
  startup_test();
  M5.Power.begin();   // handle battery charging
  start_wifi(connection);
}


// The network connection can be specified as:
//  NETWORK_CONNECTION_NONE No network connection will be made
//  NETWORK_CONNECTION_AUTO Autoconnect will select the strongest known connection
//  NETWORK_CONNECTION_UI   M5ez UI will list all favorites and will allow the user to pick one
//  The name of any defined M5ez AutoConnect network
//
void M5Sys::start_wifi(const char* connection) {
  log.verbose("M5Sys::start_wifi(%s)\n", connection);
  // Do not autoconnect while we're deciding about connection method
  ez.wifi.autoConnect = false;

  if(0 == strcmp(NETWORK_CONNECTION_NONE, connection)) {
    disconnect();
    return;
  }
  else if(0 == strcmp(NETWORK_CONNECTION_AUTO, connection)) {
    auto_connect();
    return;
  }
  else if(0 == strcmp(NETWORK_CONNECTION_UI, connection)) {
    String conn = pick_connection(nullptr);
    if(0 == conn.length()) return;
    start_wifi(conn.c_str());
    return;
  }
  else {
    // This is assumed to be the SSID of a wifi network.
    // Search all autoconnects known for a match
    for (int8_t n = 0; n < ez.wifi.networks.size(); n++) {
      if(0 == ez.wifi.networks[n].SSID.compareTo(connection)) {
        log.info("connecting to network %s\n", connection);
        ez.wifi.autoConnect = false;
        WiFi.begin(connection, ez.wifi.networks[n].key.c_str());
        return;
      }
    }
    log.error("M5Sys::start_wifi: specified network name not found in autoconnects\n");
  }
}


// Get the connection type from prefs; sow it in the connection picker
// display as selected.  Of a different selection is made, write it to
// prefs. Start wifi with the connection indicated.
//
void M5Sys::menu() {
  log.verbose("M5Sys::menu()\n");
  Preferences preferences;
  char        buffer[_M5SYSBUFFERSIZE] = { 0 };

  bool stat = preferences.begin(M5SYS_PREFS_NAME, true);  // read-only
  if(stat) {
    int sz = preferences.getString(CONNECTION_TYPE, buffer, _M5SYSBUFFERSIZE-1);
    if(0 == sz) {
      log.error("M5SysBase::start_wifi: failed to read preference conn_type\n");
    }
  }
  else {
    log.error("M5SysBase::start_wifi: failed to open M5ez\n");
  }
  preferences.end();
  
  String conn = pick_connection(buffer);
  if(0 == conn.length()) return;

  // We know we have a string. If it is different that original, set prefs
  if(0 != conn.compareTo(buffer)) {
    bool stat = preferences.begin(M5SYS_PREFS_NAME);  // read-write
    if(stat) {
      int sz = preferences.putString(CONNECTION_TYPE, conn.c_str());
      if(0 == sz) {
        log.error("M5SysBase::start_wifi: failed to write preference conn_type\n");
      }
      else {
        log.info("setting %s preference to %s\n", CONNECTION_TYPE, conn.c_str());
      }
    }
    else {
      log.error("M5SysBase::start_wifi: failed to open M5ez\n");
    }
    preferences.end();
  }
  start_wifi(conn.c_str());
}


// Connection was set to None.  Disconnect from the network.
//
void M5Sys::disconnect() {
  log.verbose("M5Sys::disconnect()\n");
  log.info("setting network connection to None\n");
  WiFi.disconnect();
  WiFi.setAutoConnect(false);
  ez.wifi.autoConnect = false;
  ez.wifi.writeFlash();
}


// Determine what preference is set in CONNECTION_TYPE.
// If it is Autoconnect, proceed with the M5ez autoconnect scheme.
// Otherwise, call back to start_wifi with the new connection type.
// There should be no danger of infinate recursion as this is only called
// for autoconnect.
//
void M5Sys::auto_connect() {
  log.verbose("M5Sys::auto_connect()\n");
  log.info("setting network connection to Autoconnect\n");
  char buffer[_M5SYSBUFFERSIZE] = { 0 };

  // Try reading M5Sys prefs, and let our autoconnect override M5ez (unless autoconnect specified)
  Preferences preferences;
  bool stat = preferences.begin(M5SYS_PREFS_NAME, true);  // read-only
  if(stat) {
    int sz = preferences.getString(CONNECTION_TYPE, buffer, _M5SYSBUFFERSIZE-1);
    if(0 == sz) {
      log.error("M5SysBase::auto_connect: failed to read preference conn_type\n");
    }
  }
  else {
    log.error("M5SysBase::auto_connect: failed to open M5ez\n");
  }
  preferences.end();

  // If we succeeded in reading from prefs, use the specified behavior
  if(strlen(buffer)) {
    log.debug("%s from prefs = %s\n", CONNECTION_TYPE, buffer);
    if(0 != strcmp(NETWORK_CONNECTION_AUTO, buffer)) {
      log.debug("recurse to start %s\n", buffer);
      // anything BUT autoconnect, recurse with a new value
      start_wifi(buffer);
      return;
    }
  }
  // use the M5ez autoConnect method
  log.debug("Using M5ez autoconnect\n");
  ez.wifi.autoConnect = true;
  ez.wifi.writeFlash();   // BUGBUG: I am not sure this should be here
}


// If currPick is nullptr, there is no current selection to indicate
// Return the name of connection picked, or an empty string
//
String M5Sys::pick_connection(const char* currPick) {
  log.verbose("M5Sys::pick_connection(%s)\n", currPick ? currPick : "(none)");
  bool  isNone  = false;
  bool  isAuto  = false;

  if(currPick) {
    isNone = (0 == strcmp(NETWORK_CONNECTION_NONE, currPick));
    isAuto = (0 == strcmp(NETWORK_CONNECTION_AUTO, currPick));
  }

  ezMenu m("Pick a Network");
  m.txtSmall();
  m.buttons("up # Back # select ## down #");
  m.addItem(isNone ? String(NETWORK_CONNECTION_NONE) + " | " + NETWORK_CONNECTION_NONE + "\tSelected" : NETWORK_CONNECTION_NONE);
  m.addItem(isAuto ? String(NETWORK_CONNECTION_AUTO) + " | " + NETWORK_CONNECTION_AUTO + "\tSelected" : NETWORK_CONNECTION_AUTO);
  for (int8_t n = 0; n < ez.wifi.networks.size(); n++) {
    String name = ez.wifi.networks[n].SSID;
    if(currPick && (0 == name.compareTo(currPick))) {
      name += " | " + name + "\tSelected";
    }
    m.addItem(name);
  }
  if(0 == m.runOnce()) return String("");
  return m.pickName();
}
