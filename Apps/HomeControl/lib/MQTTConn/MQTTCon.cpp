#include <M5Sys.h>
#include "MQTTCon.h"

extern M5Sys  m5sys;


MQTTCon::MQTTCon() : client(wifiClient) {
  VERBOSE("MQTTCon::MQTTCon()\n");
}


void MQTTCon::begin(MQTTConRec* cr){
  VERBOSE("MQTTCon::begin(MQTTConRec)\n");
  conrec = *cr;
  // BUGBUG: do we need an explicit copy ctor?
  DEBUG("client_name = %s\n", conrec.client_name.c_str());
  DEBUG("topic       = %s\n", conrec.topic.c_str());
  DEBUG("addr        = %s\n", conrec.addr.toString().c_str());
  DEBUG("port        = %d\n", conrec.port);
  DEBUG("callback    = %d\n", conrec.callback);
  client.setServer(conrec.addr, conrec.port);
  client.setCallback(conrec.callback);
  INFO("MQTT initialized\n");
}


// if idle() is a nullptr it will not be called, else it's called
// periodically while waiting.
//
bool MQTTCon::connect(void (*idle)()) {
  VERBOSE("MQTTCon::connect()\n");
  if(!WiFi.isConnected()) {
    DEBUG("MQTTCon::connect(): WiFi is not connected\n");
    while(!WiFi.isConnected()) {
      // Connection has already been initiated
      if(idle) idle();
      delay(500);
    }
  }
  VERBOSE("WiFi connected\n");
  if(!client.connected()) {
    DEBUG("waiting for client connection\n");
    while(!client.connected()) {
      if(client.connect(conrec.client_name.c_str())) break;
      if(idle) idle();
      delay(5000);
    }
    INFO("Connected to MQTT broker\n");
    client.subscribe(conrec.topic.c_str());
    INFO("Subscribed to topic %s\n", conrec.topic.c_str());
  }
  VERBOSE("client connected\n");
  return true;
}


// Keep the connection up once its been established
//
bool MQTTCon::reconnect() {
  VERBOSE("MQTTCon::reconnect()\n");
  if(!WiFi.isConnected()) {
    DEBUG("MQTTCon::reconnect: wifi not connected\n");
    return false;
  }
  VERBOSE("WiFi is connected\n");
  if(!client.connected()) {
    DEBUG("MQTTCon::reconnect: client was not connected\n");
    bool result = client.connect(conrec.client_name.c_str());
    DEBUG("MQTTCon::reconnect: client connect %s\n", result ? "succeeded" : "failed");
    if(result) {
      client.subscribe(conrec.topic.c_str());
      DEBUG("Subscribed to topic %s\n", conrec.topic.c_str());
    }
    return result;
  }
  return true;
}


void MQTTCon::loop() {
  VERBOSE("MQTTCon::loop()\n");
  if(reconnect()) {
    VERBOSE("calling client.loop()\n");
    client.loop();
  }
}


bool MQTTCon::publish(const char* topic, const char* message) {
  VERBOSE("MQTTCon::publish('%s', '%s')\n", topic, message);
  return client.publish(topic, message);
}
