#include <M5Sys.h>
#include "Cluster.h"

extern M5Sys    m5sys;


Cluster::Cluster(const char* name) {
  VERBOSE("Cluster::Cluster(%s)\n", name);
  this->name = name;
}


// Add an item with name and value if it doesn't exist.
// If item does exist, set its value.
// if value is nullptr, set value to ""
void Cluster::set_item(const char* item, const char* value) {
  VERBOSE("Cluster::set_item('%s', '%s')\n", item, value ? value : "nullptr");
  if(nullptr == value) value = "";
  int i = findItem(item);
  if(i < 0) {
    DEBUG("adding new item ['%s':'%s']\n", item, value);
    items.push_back(Item(item, value));
  }
  else {
    DEBUG("updating item '%s' to %s\n", item, value);
    items[i].set_value(value);
  }
}


const char* Cluster::get_name() {
  VERBOSE("Cluster::get_name()\n");
  return name.c_str();
}


// return -1 if the item is not found, else its index
//
int Cluster::findItem(const char* name) {
  VERBOSE("Cluster::findItem(%s)\n", name);
  for(int i = 0; i < items.size(); i++) {
    const char* item_name = items[i].get_name();
    if(item_name) {
      if(0 == strcmp(name, item_name)) {
        DEBUG("Cluster::findItem: match found at index %d\n", i);
        return i;
      }
    }
  }
  DEBUG("Cluster::findItem: no match for %s found\n", name);
  return -1;
}


Item* Cluster::get_item(int index) {
  VERBOSE("Cluster::get_item(%d)\n", index);
  if(index < items.size()) return &items[index];
  else return nullptr;
}


Item* Cluster::get_item(const char* name) {
  VERBOSE("Cluster::get_item(%s)\n", name);
  for(int i = 0; i < items.size(); i++) {
    const char* item_name = items[i].get_name();
    if(item_name) {
      if(0 == strcmp(name, item_name)) {
        DEBUG("Cluster::findItem: match found at index %d\n", i);
        return &items[i];
      }
    }
  }
  return nullptr;
}


// Determine if the cluster has gone offline by seeing if all of
// its items haven't reported in in 'seconds'. MQTT refreshes
// timing of each element as it reports, which should be refreshed
// every 30 to 60 seconds if communications are good.
// Return true if all items are older than 'seconds' old and the
// cluster should be deleted.
//
bool Cluster::older_than(int seconds) {
  VERBOSE("Cluster::older_than(%d) for cluster '%s'\n", seconds, name.c_str());
  unsigned long limit = millis() - (seconds * 1000);
  for(int i = 0; i < items.size(); i++) {
    if(items[i].get_time() > limit) return false;
  }
  return true;
}
