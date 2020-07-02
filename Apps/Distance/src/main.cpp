// The purpose of this program is to determine the surface distance between two points
// on the surface of the Earth, identified by latitude and longitude.
// One or both can be Locations, selected from m5sys.

#include <M5Sys.h>

#define     R_KM            6371
#define     R_MI            3958.8
#define     TO_DEG          (180.0 / 3.1415926536)
#define     TO_RAD          (3.1415926536 / 180.0)
#define     HOME_LAT        47.72544735
#define     HOME_LON        -122.1802621

M5Sys       m5sys;
bool        sited           = false;  // True if a home location is selected, else false
Location_t* home_location   = nullptr;
Location_t* dest_location   = nullptr;
double      dest_latitude   = 0.0;
double      dest_longitude  = 0.0;


// Return distance between two lat/lon pairs in miles.
double haversine(double lat1, double lon1, double lat2, double lon2) {
  VERBOSE("haversine(%lf/%lf -> %lf/%lf)\n", lat1, lon1, lat2, lon2);
  double dx, dy, dz;
  lon1 -= lon2;
  lon1 *= TO_RAD, lat1 *= TO_RAD, lat2 *= TO_RAD;
 
  dz = sin(lat1) - sin(lat2);
  dx = cos(lon1) * cos(lat1) - cos(lat2);
  dy = sin(lon1) * cos(lat1);
  return (asin(sqrt(dx * dx + dy * dy + dz * dz) / 2) * 2 * R_MI);
}


// Return the heading from one lat/lon to another in degrees
// From https://www.igismap.com/formula-to-find-bearing-or-heading-angle-between-two-points-latitude-longitude
double bearing_to(double lat1, double lon1, double lat2, double lon2) {
  VERBOSE("bearing_to(%lf/%lf -> %lf/%lf)\n", lat1, lon1, lat2, lon2);
  lon1 *= TO_RAD, lat1 *= TO_RAD, lon2 *= TO_RAD, lat2 *= TO_RAD;
  double dLon     = (lon2 - lon1);
  double y        = sin(dLon) * cos(lat2);
  double x        = cos(lat1) * sin(lat2) - sin(lat1) * cos(lat2) * cos(dLon);
  double heading  = atan2(y, x);

  heading = heading * TO_DEG;   // convert radians to degrees
  return heading;
}


// Truncate trailing zeros AFTER the decimal point.
// Return same value passed in, for fluency
//
char* trim_trailing_zeros(char* buffer) {
  if(strstr(buffer, ".")) {
    int i = strlen(buffer) - 1;
    // delete trailing zeros until down to 1 character, or until '.' is next trailing character
    while(i-- > 0 && '0' == buffer[i] && '.' != buffer[i-1]) buffer[i] = '\0';
  }
  return buffer;
}


void calculate() {
  VERBOSE("calculate()\n");
  double miles    = haversine(home_location->latitude, home_location->longitude, dest_latitude, dest_longitude);
  double heading  = bearing_to(home_location->latitude, home_location->longitude, dest_latitude, dest_longitude);
  ez.msgBox("Distance", String("Distance is " + String(miles) + " miles.\nAzimuth is " + String(heading) + " degrees."));
}


void get_location() {
  VERBOSE("get_location()\n");
  dest_location = m5sys.position.pick_location();
  if(dest_location) {
    DEBUG("Selected location %s\n", dest_location->name);
    dest_latitude  = dest_location->latitude;
    dest_longitude = dest_location->longitude;
  }
  else {
    DEBUG("No destination location selected\n");
  }
}


void get_latitude() {
  VERBOSE("get_latitude()\n");
  char buffer[32];
  sprintf(buffer, "%f", dest_latitude);
  trim_trailing_zeros(buffer);
  String s = ez.textInput("Enter Target Latitude", buffer);
  sscanf(s.c_str(), "%lf", &dest_latitude);
}


void get_longitude() {
  VERBOSE("get_longitude()\n");
  char buffer[32];
  sprintf(buffer, "%f", dest_longitude);
  trim_trailing_zeros(buffer);
  String s = ez.textInput("Enter Target Longitude", buffer);
  sscanf(s.c_str(), "%lf", &dest_longitude);
}


void setup() {
  m5sys.begin("Distance", NETWORK_CONNECTION_NONE);
  home_location = m5sys.position.get_current();
  sited = nullptr != home_location;
}


void loop() {
  VERBOSE("loop()\n");
  ezMenu m(String("Distance") + ((sited) ? " from " + home_location->name : "s"));
  m.txtSmall();
  m.buttons("up # back | Home # select ## down #");
  if(!sited)
    m.addItem("Select Home Position");
  else {
    m.addItem("Select destination location",    get_location);
    m.addItem("Select destination latitude",    get_latitude);
    m.addItem("Select destination longitude",   get_longitude);
    m.addItem("Calculate Distance and Azimuth", calculate);
  }
  m.run();
  m5sys.goHome();
}
