#pragma once

#include <Arduino.h>
#include <Logger.h>

struct Location_t {
  Location_t(const char* name = "", double latitude = 0.0, double longitude = 0.0, int altitude = 0) {
    this->name      = name;
    this->latitude  = latitude;
    this->longitude = longitude;
    this->altitude  = altitude;
  }
  String  name;
  double  latitude;
  double  longitude;
  int     altitude;    // Unit: meters
};


class Locations {
  public:
    static void                     begin(Logger* logger);
    static void                     add(String name, double lat, double lon, int alt=0);
    static bool                     remove(int8_t index);
    static bool                     remove(String name);
    static int8_t                   indexForName(String name);
    static void                     readFlash();
    static void                     writeFlash();
    static void                     menu();
    static Location_t* const        get_current();
    static Location_t* const        pick_location();
    static double                   distance(Location_t* origin, Location_t* destination);
    static double                   bearing (Location_t* origin, Location_t* destination);
    static std::vector<Location_t>  locations;
  private:
    static void                     _clear_locations();
    static void                     _create_location();
    static void                     _delete_location();
    static void                     _select_location();
    static void                     _inspect_location(String name);
    static String                   _current_location;
    static Logger*                  _log;
};
