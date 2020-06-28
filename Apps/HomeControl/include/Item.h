#pragma once

#include <Arduino.h>


class Item {
  public:
    Item(const char* name);
    Item(const char* name, const char* value);
    void          set_value(const char* value);
    const char*   get_name();
    const char*   get_value();
    unsigned long get_time();
  protected:
    String        name;
    String        value;
    unsigned long time; // millis() when updated
};
