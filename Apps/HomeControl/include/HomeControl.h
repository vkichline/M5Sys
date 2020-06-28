#pragma once

#include "Cluster.h"


class HomeControl {
  public:
    bool                  process_mqtt_message(char* topic, byte* payload, unsigned int length);
    void                  set_sensor(const char* name, const char* item, const char* value);
    void                  set_controller(const char* name, const char* item, const char* value);
    Cluster*              get_sensor(int index);
    Cluster*              get_sensor(const char* name);
    Cluster*              get_controller(int index);
    Cluster*              get_controller(const char* name);
    int                   purge_clusters_older_than(int seconds);
  protected:
    void set_data(std::vector<Cluster>& vec, const char* name, const char* item, const char* value);
    std::vector<Cluster>  sensors;
    std::vector<Cluster>  controllers;
};
