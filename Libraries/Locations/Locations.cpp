#include <Preferences.h>
#include <M5ez.h>
#include "Locations.h"

#define M5SYS_PREFS_NAME  "M5Sys"


std::vector<Location_t>   Locations::locations;
Logger*                   Locations::_log;
String                    Locations::_current_location = "";


void Locations::begin(Logger* logger) {
  _log = logger;
  _log->verbose("Locations::begin()\n");
	readFlash();
}


void Locations::add(String name, double lat, double lon, int alt) {
  _log->verbose("Locations::add(%s, %.4f, %.4f, %d)\n", name.c_str(), lat, lon, alt);
  Location_t          new_loc;
  new_loc.name      = name;
  new_loc.latitude  = lat;
  new_loc.longitude = lon;
  new_loc.altitude  = alt;
  locations.push_back(new_loc);
  writeFlash();
}


bool Locations::remove(int8_t index) {
  _log->verbose("Locations::remove(%d)\n", index);
  if(index < 0 || index >= locations.size()) return false;
  locations.erase(locations.begin() + index);
  writeFlash();
  return true;
}


bool Locations::remove(String name) {
  _log->verbose("Locations::remove(%s)\n", name);
  return remove(indexForName(name));
}


int8_t Locations::indexForName(String name) {
  _log->verbose("Locations::indexForName(%s)\n", name);
  for (uint8_t n = 0; n < locations.size(); n++) {
    if (locations[n].name == name) return n;
  }
  return -1;
}


void Locations::readFlash() {
  _log->verbose("Locations::readFlash()\n");
  Preferences prefs;
  Location_t  new_loc;
  String      idx;
  uint8_t     index = 1;

  locations.clear();
  prefs.begin(M5SYS_PREFS_NAME, true);	// true: read-only
  _current_location = prefs.getString("LOC0", "");
  while (true) {
    idx =  "LOC" + (String)index;
    String name  = prefs.getString(idx.c_str(), "");
    idx =  "LAT" + (String)index;
    double lat   = prefs.getDouble(idx.c_str(), 100.0);  // 100 is an impossible value
    idx =  "LON" + (String)index;
    double lon   = prefs.getDouble(idx.c_str(), 0.0);
    idx =  "ALT" + (String)index;
    int    alt   = prefs.getInt(idx.c_str(), 0);
    if (name != "") {
      new_loc.name      = name;
      new_loc.latitude  = lat;
      new_loc.longitude = lon;
      new_loc.altitude  = alt;
      locations.push_back(new_loc);
      index++;
    }
    else {
      break;
    }
  }
  prefs.end();
}


void Locations::writeFlash() {
  _log->verbose("Locations::writeFlash()\n");
  Preferences prefs;
  prefs.begin(M5SYS_PREFS_NAME, false);           // false: read-write
  String idx;

  _clear_locations();
  prefs.putString("LOC0", _current_location);     // currently selected location
  for(uint8_t n = 0; n < locations.size(); n++) {
    idx = "LOC" + (String)(n + 1);
    prefs.putString(idx.c_str(), locations[n].name);
    idx = "LAT" + (String)(n + 1);
    prefs.putDouble(idx.c_str(), locations[n].latitude);
    idx = "LON" + (String)(n + 1);
    prefs.putDouble(idx.c_str(), locations[n].longitude);
    idx = "ALT" + (String)(n + 1);
    prefs.putInt(idx.c_str(), locations[n].altitude);
  }
  prefs.end();
}


void Locations::_clear_locations() {
  _log->verbose("Locations::_clear_locations()\n");
  Preferences prefs;
  String      idx;
  uint8_t     n = 1;

  prefs.begin(M5SYS_PREFS_NAME, false); // false: read-write
  prefs.remove("LOC0");                 // currently selected location
  while(true) {
    idx = "LOC" + (String)n;
    bool ok = prefs.remove(idx.c_str());
    if(!ok) {
      break;
    }
    else {
      idx = "LAT" + (String)n;
      prefs.remove(idx.c_str());
      idx = "LON" + (String)n;
      prefs.remove(idx.c_str());
      idx = "ALT" + (String)n;
      prefs.remove(idx.c_str());
    }
    n++;
  }
  prefs.end();
}


Location_t* const Locations::get_current() {
  _log->verbose("Locations::get_current()\n");
  if(0 == _current_location.length()) return nullptr;
  int index = indexForName(_current_location);
  if(-1 == index) return nullptr;
  return &locations[index];
}


void Locations::menu() {
  _log->verbose("Locations::menu()\n");
  ezMenu  m("Locations");
  m.txtSmall();
  m.buttons("up # Back # select ## down #");
  m.addItem(String("Current:\t") + ((0 == _current_location.length()) ? "None" : _current_location), Locations::_select_location);
  m.addItem("Create Location", Locations::_create_location);
  m.addItem("Delete Location", Locations::_delete_location);
  while(true) {
    if(0 == m.runOnce()) return;
    // See if the current setting has changed
    m.setCaption(1, String("Current:\t") + ((0 == _current_location.length()) ? "None" : _current_location));
  }
}


Location_t* const Locations::pick_location() {
  _log->verbose("Locations::pick_location()\n");
  if(0 == locations.size()) return nullptr; // cannot display a menu with no items
  ezMenu m("Pick a Location");
  m.txtSmall();
  m.buttons("up # Back # select ## down #");
  for(uint8_t n = 0; n < locations.size(); n++) {
    m.addItem(locations[n].name);
  }
  if(0 == m.runOnce()) return nullptr;
  int8_t index = indexForName(m.pickName());
  if(0 > index) return nullptr;
  return &locations[index];
}


void Locations::_create_location() {
  _log->verbose("Locations::_create_location()\n");
  String name, temp;
  double lat  = 0.0;
  double lon  = 0.0;
  int    alt  = 0;

  ezMenu m("Create Location");
  m.txtSmall();
  m.buttons("up # Back # select ## down #");
  m.addItem("Name");
  m.addItem("Latitude");
  m.addItem("Longitude");
  m.addItem("Altitude (m)");
  while(true) {
    if(0 == m.runOnce()) {
      if(0 < name.length()) {
        remove(name);               // if there's already a location with this name, eliminate
        add(name, lat, lon, alt);
      }
      return;
    }
    String result = m.pickName();
    _log->debug("Locations::_create_location(): result = %s\n", result.c_str());
    if(0 == result.compareTo("Name")) {
      name = ez.textInput("Specify Name", name);
      m.setCaption("Name", String("Name\t") + name);
    }
    else if(0 == result.compareTo("Latitude")) {
      temp = ez.textInput("Specify Latitude", String(lat));
      lat  = atof(temp.c_str());
      m.setCaption("Latitude", String("Latitude\t") + String(lat));
    }
    else if(0 == result.compareTo("Longitude")) {
      temp = ez.textInput("Specify Longitude", String(lon));
      lon  = atof(temp.c_str());
      m.setCaption("Longitude", String("Longitude\t") + String(lon));
    }
    else if(0 == result.compareTo("Altitude (m)")) {
      temp = ez.textInput("Specify Altitude", String(alt));
      alt  = atoi(temp.c_str());
      m.setCaption("Altitude (m)", String("Altitude (m)\t") + String(alt));
    }
  }
}


void Locations::_select_location() {
  _log->verbose("Locations::_select_location()\n");
  if(0 == locations.size()) return; // cannot display a menu with no items
  ezMenu m("Select Location");
  m.txtSmall();
  m.buttons("up # Back # sel # insp # down #");
  for(uint8_t n = 0; n < locations.size(); n++) {
    m.addItem(locations[n].name);
  }
  if(0 == m.runOnce()) return;
  String button = m.pickButton();
  _log->info("button = %s\n", button);
  if(0 == button.compareTo("sel")) {
    _current_location = m.pickName();
    writeFlash();
  }
  else if(0 == button.compareTo("insp")) 
    _inspect_location(m.pickName());
}


void Locations::_delete_location() {
  _log->verbose("Locations::_delete_location()\n");
  if(0 == locations.size()) return; // cannot display a menu with no items
  ezMenu m("Delete a Location");
  m.txtSmall();
  m.buttons("up # Back # select ## down #");
  for(uint8_t n = 0; n < locations.size(); n++) {
    m.addItem(locations[n].name);
  }
  if(0 == m.runOnce()) return;
  String name = m.pickName();
  _log->info("name = %s\n", name);
  String message = "Delete \"";
  message.concat(name);
  message.concat("\": Are you sure?");
  String result = ez.msgBox("Delete Location", message, "Cancel ## OK");
  _log->info("result = %s\n", result);
  if(0 == result.compareTo("OK")) {
    if(0 == name.compareTo(_current_location))
      _current_location = "";
    remove(name);
  }
}


void Locations::_inspect_location(String name) {
  _log->verbose("Locations::_inspect_location(%s)\n", name.c_str());
  int index = indexForName(name);
  if(-1 == index) return;

  ez.header.show("Location: " + name);
  ez.buttons.show("OK");
  ez.canvas.clear();
  ez.canvas.font(&FreeSans9pt7b);
  ez.canvas.lmargin(8);
  ez.canvas.y(28);
  ez.canvas.print("Name");
  ez.canvas.x(160);
  ez.canvas.println(locations[index].name);
  ez.canvas.print("Latitude");
  ez.canvas.x(160);
  ez.canvas.printf("%.7f", locations[index].latitude);
  ez.canvas.println();
  ez.canvas.print("Longitude");
  ez.canvas.x(160);
  ez.canvas.printf("%.5f", locations[index].longitude);
  ez.canvas.println();
  ez.canvas.print("Altitude");
  ez.canvas.x(160);
  ez.canvas.println(locations[index].altitude);
  ez.buttons.wait();
}
