//  Settings program for the M5Stack
//  Van Kichline
//  May, in the year of the plague

#include <SD.h>
#include <M5Sys.h>
#include <SysInfo.h>
#include <M5NeoPixels.h>

M5Sys       m5sys;
M5NeoPixels pix;
ezMenu      topMenu("M5Sys Home");  // So event can update caption


////////////////////////////////////////////////////////////////////////////////
//
//  Load Program form SD Card Menu Item
//
////////////////////////////////////////////////////////////////////////////////

bool ascending_alpha_comp(const char* s1, const char* s2)                   { return 0 > strcmp(s1, s2); }
bool ascending_alpha_case_insensitive_comp(const char* s1, const char* s2)  { return 0 > strcasecmp(s1, s2); }

// Currently, apps end in '.bin' and are stored at the root of the SD card.
// This is subject to change.
//
void load_program_menu() {
  VERBOSE("program_control()\n");
  File root = SD.open("/");
  if(!root) {
    ez.msgBox("Critical Error", "SD Card not found.");
    return;
  }
  ezMenu m("Load Program");
  m.txtSmall();
  m.buttons("up # Back # select ## down #");
  m.setSortFunction(ascending_alpha_case_insensitive_comp);

  // Fill the menu with executable binaries:
  while(true) {
    File f = root.openNextFile();
    if(!f) break;
    if(!f.isDirectory()) {
      String name(f.name());
      DEBUG("%s\n", name.c_str());
      if('/' == name.charAt(0)) name.remove(0, 1);  // Remove the leading '/'
      // Invisible (resource fork) files can be eliminated with MacZap, but if not, skip them!
      if(!name.startsWith("._")) {
        if(name.endsWith(".bin")) {
          name.remove(name.length() - 4);           // remove '.bin'
          if(0 != name.compareTo("menu")) {         // don't show the launcher (which is running)
            m.addItem(name);
          }
        }
      }
    }
    f.close();
  }
  root.close();
  switch(m.runOnce()) {
    case 0:   return;
    default:  m5sys.launch(m.pickName().c_str());
  }
}


////////////////////////////////////////////////////////////////////////////////
//
//  M5NeoPixels Menu
//
////////////////////////////////////////////////////////////////////////////////

void all_on_white() { pix.all(pix.np_white); }

void neopixels_menu() {
  VERBOSE("neopixels_menu()\n");
  pix.begin();
  ezMenu m("Neopixles");
  m.txtSmall();
  m.buttons("up # Back # select ## down #");
  m.addItem("Clear");
  m.addItem("On");
  m.addItem("Red/Green");
  m.addItem("Blue/Red");
  while(true) {
    if(0 == m.runOnce()) return;
    String result = m.pickName();
    DEBUG("neopixels_menu(): result = %s\n", result.c_str());
    if(     0 == result.compareTo("Clear"))      pix.clear();
    else if(0 == result.compareTo("On"))         pix.all(pix.np_white);
    else if(0 == result.compareTo("Red/Green"))  { pix.left(pix.np_red);  pix.right(pix.np_green); }
    else if(0 == result.compareTo("Blue/Red"))   { pix.left(pix.np_blue); pix.right(pix.np_red);   }
  }
}


////////////////////////////////////////////////////////////////////////////////
//
//  System Settings Menu Item
//
////////////////////////////////////////////////////////////////////////////////

void system_settings_menu() {
  VERBOSE("settings_control()\n");
  ezMenu m("Settings");
  m.txtSmall();
  m.buttons("up # Back # select ## down #");
  m.addItem("Autoconnect",  ez.wifi.menu);
  m.addItem("Backlight",    ez.backlight.menu);
  m.addItem("Battery",      ez.battery.menu);
  m.addItem("BLE Settings", ez.ble.menu);
  m.addItem("Clock",        ez.clock.menu);
  m.addItem("Location",     m5sys.position.menu);
  m.addItem("Logging",      m5sys.log.menu);
  m.addItem("NeoPixels",    neopixels_menu);
  m.addItem("Theme",        ez.theme->menu);
  m.run();
}


////////////////////////////////////////////////////////////////////////////////
//
//  WiFi Selection Menu Item
//
////////////////////////////////////////////////////////////////////////////////

void wifi_selection_menu() {
  VERBOSE("network_control()\n");
  m5sys.menu();
}


////////////////////////////////////////////////////////////////////////////////
//
//  System Info Menu Item
//
////////////////////////////////////////////////////////////////////////////////

void system_info_menu() {
  VERBOSE("sysinfo_control()\n");
  SysInfo sysinfo;
  sysinfo.display_info(); // This loops forever or loads Home
}


////////////////////////////////////////////////////////////////////////////////
//
//  Power Menu Item
//
////////////////////////////////////////////////////////////////////////////////

void power_menu() {
  VERBOSE("power_control()\n");
  String str = ez.msgBox("Power", "Are you sure you want to shutdown the M5Sys?", "No ## Yes");
  if(String("Yes") == str) {
    INFO("power_control: shutdown\n");
    M5.Power.deepSleep();
  }
}


////////////////////////////////////////////////////////////////////////////////
//
//  Arduino Program Control: setup() and loop()
//
////////////////////////////////////////////////////////////////////////////////

String lastKnownConnection = "";

// This is intended to update the menu to show the current wifi connection
uint16_t update_connection_menu() {
  if(WiFi.isConnected()) {
    String ssid = WiFi.SSID();
    if(0 != ssid.compareTo(lastKnownConnection)) {
      lastKnownConnection = ssid;
      topMenu.setCaption(2, "Connected to " + ssid);
    }
  }
  else {
    if(0 < lastKnownConnection.length()) {
      lastKnownConnection = "";
      topMenu.setCaption(2, "No WiFi Connection");
    }
  }
  return 1000;
}

void setup() { m5sys.begin("M5Home", NETWORK_CONNECTION_AUTO); }

void loop() {
  VERBOSE("Loop()\n");  // Not always a good idea, but this loop doesn't repeat
  topMenu.txtSmall();

  topMenu.addItem("Load Program from SD Card",  load_program_menu);
  topMenu.addItem("No WiFi Connection",         wifi_selection_menu);
  topMenu.addItem("System Settings",            system_settings_menu);
  topMenu.addItem("System Info",                system_info_menu);
  topMenu.addItem("Power",                      power_menu);
  ez.addEvent(update_connection_menu);
  topMenu.run();
}
