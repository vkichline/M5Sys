#pragma once

#include <WiFi.h>
#include <PubSubClient.h>

struct MQTTConRec {
  String      client_name;
  String      topic;
  IPAddress   addr;
  int         port;
  void        (*callback)(char* topic, byte* payload, unsigned int length);
};


class MQTTCon {
  public:
    MQTTCon();
    void          begin(MQTTConRec* conrec);
    bool          connect(void (*idle)() = nullptr);
    bool          reconnect();
    void          loop();
    bool          publish(const char* topic, const char* message);
  protected:
    MQTTConRec    conrec;
    WiFiClient    wifiClient;
    PubSubClient  client;
};

