#pragma once

#include <Arduino.h>
#include <Item.h>

// A cluster is a group of individual sensors or controlers (Items)
// A cluster has a name (its location) and a list of items

class Cluster {
  public:
    Cluster(const char* name);
    const char*       get_name();
    void              set_item(const char* name, const char* value = nullptr);
    Item*             get_item(int index);
    Item*             get_item(const char* name);
    int               findItem(const char* name);
    bool              older_than(int seconds);
  protected:
    std::vector<Item> items;
    String            name;
};
