#include <M5Sys.h>
#include "Item.h"

extern M5Sys  m5sys;


Item::Item(const char* name) {
  VERBOSE("Item::Item(%s)\n", name);
  this->name  = name;
  value       = "";
  time        = millis();
}


Item::Item(const char* name, const char* value) {
  VERBOSE("Item::Item('%s', '%s)\n", name, value);
  this->name  = name;
  this->value = value;
  time        = millis();
}


void  Item::set_value(const char* value) {
  VERBOSE("Item::set_value(%s) for %s\n", value, name.c_str());
  this->value = value;
  time        = millis();
}


const char* Item::get_name() {
  VERBOSE("Item::get_name() for %s\n", name.c_str());
  return name.c_str();
}


const char* Item::get_value() {
  VERBOSE("Item::get_value() for %s is %s\n", name.c_str(), value.c_str());
  return value.c_str();
}


unsigned long Item::get_time() {
  VERBOSE("Item::get_time() for %s is %llu\n", name.c_str(), time);
  return time;
}
