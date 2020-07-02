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
#define     HOME_ALT        83

M5Sys       m5sys;
bool        sited           = false;  // True if a home location is selected, else false
Location_t* home_location   = nullptr;
Location_t* dest_location   = new Location_t;


// Return distance between two locations in miles.
//
double distance(Location_t* origin, Location_t* destination) {
  VERBOSE("distance(%lf/%lf -> %lf/%lf)\n", origin->latitude, origin->longitude, destination->latitude, destination->longitude);
  double dx, dy, dz;
  double delta = (origin->longitude - destination->longitude) * TO_RAD;
  double lat1  = origin->latitude * TO_RAD;
  double lat2  = destination->latitude * TO_RAD;
 
  dz = sin(lat1) - sin(lat2);
  dx = cos(delta) * cos(lat1) - cos(lat2);
  dy = sin(delta) * cos(lat1);
  return (asin(sqrt(dx * dx + dy * dy + dz * dz) / 2) * 2 * R_MI);
}


// Return the heading from one location to another in degrees
// From https://www.igismap.com/formula-to-find-bearing-or-heading-angle-between-two-points-latitude-longitude
//
double bearing(Location_t* origin, Location_t* destination) {
  VERBOSE("bearing_to(%lf/%lf -> %lf/%lf)\n", origin->latitude, origin->longitude, destination->latitude, destination->longitude);
  double lon1     = origin->longitude      * TO_RAD;
  double lat1     = origin->latitude       * TO_RAD;
  double lon2     = destination->longitude * TO_RAD;
  double lat2     = destination->latitude  * TO_RAD;
  double delta    = lon2 - lon1;
  double y        = sin(delta) * cos(lat2);
  double x        = cos(lat1)  * sin(lat2) - sin(lat1) * cos(lat2) * cos(delta);
  double heading  = atan2(y, x);
  return heading  * TO_DEG;       // convert radians to degrees
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
  double miles    = distance(home_location, dest_location);
  double heading  = bearing(home_location, dest_location);
  ez.msgBox("Distance", String("Distance is " + String(miles) + " miles.\nAzimuth is " + String(heading) + " degrees."));
}


void get_origin() {
  VERBOSE("get_origin()\n");
  home_location = m5sys.position.pick_location();
  DEBUG("Selected location %.4f, %.4f\n", dest_location->latitude, dest_location->longitude);
}


void get_destination() {
  VERBOSE("get_destination()\n");
  dest_location = m5sys.position.pick_location();
  DEBUG("Selected location %.4f, %.4f\n", dest_location->latitude, dest_location->longitude);
}


void get_latitude() {
  VERBOSE("get_latitude()\n");
  char buffer[32];
  sprintf(buffer, "%f", dest_location->latitude);
  trim_trailing_zeros(buffer);
  String s = ez.textInput("Enter Target Latitude", buffer);
  sscanf(s.c_str(), "%lf", &dest_location->latitude);
}


void get_longitude() {
  VERBOSE("get_longitude()\n");
  char buffer[32];
  sprintf(buffer, "%f", dest_location->longitude);
  trim_trailing_zeros(buffer);
  String s = ez.textInput("Enter Target Longitude", buffer);
  sscanf(s.c_str(), "%lf", &dest_location->longitude);
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
    m.addItem("Select Home Position",           get_origin);
  else {
    m.addItem("Select destination location",    get_destination);
    m.addItem("Select destination latitude",    get_latitude);
    m.addItem("Select destination longitude",   get_longitude);
    m.addItem("Calculate Distance and Azimuth", calculate);
  }
  while(true) {
    if(0 == m.runOnce()) m5sys.goHome();
    String result = m.pickName();
    if(0 == result.compareTo("Select Home Position")) {
      m.deleteItem(1);
      m.addItem("Select destination location",    get_destination);
      m.addItem("Select destination latitude",    get_latitude);
      m.addItem("Select destination longitude",   get_longitude);
      m.addItem("Calculate Distance and Azimuth", calculate);
    }
  }
}
