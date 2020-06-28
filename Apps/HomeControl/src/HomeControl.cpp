#include <M5Sys.h>
#include "HomeControl.h"

#define   SENSOR_KIND   "sensors"
#define   CONTROL_KIND  "status"

extern    M5Sys         m5sys;


// NOTE that process_mqtt_message modifies topic in place using strtok to divide
// it into three separate strings. topic appears truncated upon return.
// Returns false if the message is rejected.
// I expect topics to break up into three segments, divided by '/'
// First is message kind: sensor, status, (other? command?)
// Second is location: Garage, Living Room, etc. (can contain spaces)
// Third is item. temp, hum Lamp, etc.
//
bool HomeControl::process_mqtt_message(char* topic, byte* payload, unsigned int length) {
  VERBOSE("HomeControl::process_mqtt_message(%s, payload, %d)\n", topic, length);
  char* buffer = (char*)malloc(length + 1);
  for(int i = 0; i < length; i++) buffer[i] = payload[i];
  buffer[length] = '\0';
  DEBUG("[%s] %s\n", topic, buffer);
  char* kind  = strtok(topic, "/");   if(nullptr == kind) return false;
  char* loc   = strtok(nullptr, "/"); if(nullptr == loc ) return false;
  char* item  = strtok(nullptr, "/"); if(nullptr == item) return false;
  char* end   = strtok(nullptr, "/"); if(nullptr != end ) return false;
  DEBUG("classified topic as kind = %s, loc = %s, item = %s\n", kind, loc, item);
  if(     0 == strcmp(SENSOR_KIND,  kind)) set_sensor(    loc, item, buffer);
  else if(0 == strcmp(CONTROL_KIND, kind)) set_controller(loc, item, buffer);
  free(buffer);
  return true;
}


void HomeControl::set_data(std::vector<Cluster>& vec, const char* name, const char* item, const char* value) {
  VERBOSE("HomeControl::set_data(vector, '%s', '%s', '%s')\n", name, item, value);
  // see if the Cluster is already in the vector
  for (int16_t i = 0; i < vec.size(); i++) {
	  if(0 == strcmp(name, vec[i].get_name())) {
      DEBUG("updating item '%s' to %s\n", item, value);
      vec[i].set_item(item, value);
      return;
		}
  }
  INFO("Adding new cluster '%s'\n", name);
  Cluster cluster(name);
  cluster.set_item(item, value);
  INFO("Adding new item '%s' to cluster '%s'\n", item, name);
  vec.push_back(cluster);
}


void HomeControl::set_sensor(const char* name, const char* item, const char* value) {
  VERBOSE("HomeControl::set_sensor('%s', '%s', '%s')\n", name, item, value);
  set_data(sensors, name, item, value);
}


void HomeControl::set_controller(const char* name, const char* item, const char* value) {
  VERBOSE("HomeControl::set_controller('%s', '%s', '%s')\n", name, item, value);
  set_data(controllers, name, item, value);
}


Cluster* HomeControl::get_sensor(int index) {
  VERBOSE("HomeControl::get_sensor(%d)\n", index);
  if(index < sensors.size()) return &sensors[index];
  return nullptr;
}


Cluster* HomeControl::get_sensor(const char* name) {
  VERBOSE("HomeControl::get_sensor(%s)\n", name);
  for(int i = 0; i < sensors.size(); i++) {
    if(0 == strcmp(name, sensors[i].get_name())) return &sensors[i];
  }
  return nullptr;
}


Cluster* HomeControl::get_controller(int index) {
  VERBOSE("HomeControl::get_controller(%d)\n", index);
  if(index < controllers.size()) return &controllers[index];
  return nullptr;
}


Cluster* HomeControl::get_controller(const char* name) {
  VERBOSE("HomeControl::get_controller(%s)\n", name);
  for(int i = 0; i < sensors.size(); i++) {
    if(0 == strcmp(name, controllers[i].get_name())) return &controllers[i];
  }
  return nullptr;
}


// Periodically, call this to purge clusters that have gone offline.
// If they report in later, they will appear again.
// Return how many clusters were purged.
//
int HomeControl::purge_clusters_older_than(int seconds) {
  VERBOSE("HomeControl::purge_clusters_older_than(%d)\n", seconds);
  int count = 0;
  // walk the list in reverse order, so deletes don't cause us to skip clusters
  for(int i = sensors.size() - 1; i >= 0 ; i--) {
    if(sensors[i].older_than(seconds)) {
      INFO("deleting sensor cluster '%s' which is more than %d seconds stale.\n", sensors[i].get_name(), seconds);
      sensors.erase(sensors.begin() + i);
      count++;
    }
    else {
      DEBUG("sensor cluster %s not deleted.\n", sensors[i].get_name());
    }
  }
  for(int i = controllers.size() - 1; i >= 0 ; i--) {
    if(controllers[i].older_than(seconds)) {
      INFO("deleting controller cluster '%s' which is more than %d seconds stale.\n", controllers[i].get_name(), seconds);
      controllers.erase(controllers.begin() + i);
      count++;
    }
    else {
      DEBUG("controller cluster %s not deleted.\n", controllers[i].get_name());
    }
  }
  return count;
}
