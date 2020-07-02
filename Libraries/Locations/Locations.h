#pragma once

#include <Arduino.h>
#include <Logger.h>

struct Location_t {
  String  name;
  float   latitude;
  float   longitude;
  int     altitude;   // Unit: meters
};


class Locations {
  public:
    static void                     begin(Logger* logger);
    static void                     add(String name, float lat, float lon, int alt=0);
    static bool                     remove(int8_t index);
    static bool                     remove(String name);
    static int8_t                   indexForName(String name);
    static void                     readFlash();
    static void                     writeFlash();
    static void                     menu();
    static Location_t* const        get_current();
    static Location_t* const        pick_location();
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
