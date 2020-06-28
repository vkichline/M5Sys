#include <M5Sys.h>
#include <PubSubClient.h>
#include <Preferences.h>

M5Sys         m5sys;
WiFiClient    wifiClient;
PubSubClient  client(wifiClient);
Preferences   preferences;

IPAddress     broker_ip(192, 168, 254, 14);
int           broker_port = 1883;
const char*   client_name = "M5SysClient";
const char*   mqtt_topic  = "#";           // Subscribe to everything


// Return false if no important button was pressed.
// If the Home button is true, exit the program and load Home
// if Change was pressed, return true, indicating that the
// network and/or broker should be changed.
//
bool check_for_exit() {
  // Don't log this function
  String result = ez.buttons.poll();
  if(0 == result.compareTo("Home")) {
    INFO("Exiting MQTTMonitor\n");
    m5sys.goHome();
    return true;  // ha ha, not really
  }
  if(0 == result.compareTo("Change")) {
    INFO("Settings change requested\n");
    return true;  // this time I mean it
  }
  return false;   // nothing to see here.
}


bool connect_to_mqtt() {
  VERBOSE("connect_to_mqtt()\n");
  if(!WiFi.isConnected()) return false;
  if(!client.connected()) {
    while(!client.connected()) {
      client.connect(client_name);
      DEBUG("Connecting to MQTT\n");
      for(int wait = 0; wait < 500; wait++) {
        if(check_for_exit()) {
          INFO("user request for settings change detected");
          client.disconnect();
          return false;  // Settings change requested.
        }
        delay(10);
      }
    }
    INFO("Connected to MQTT\n");
    ez.canvas.print("Connected to broker as ");
    ez.canvas.println(client_name);
    client.subscribe(mqtt_topic);
    INFO("Subscribed to topic %s\n", mqtt_topic);
    ez.canvas.print("Subscribed to topic ");
    ez.canvas.println(mqtt_topic);
  }
  return true;
}


void callback(char* topic, byte* payload, unsigned int length) {
  VERBOSE("callback(%s, payload)\n", topic);
  char* buffer = (char*)malloc(length + 1);
  for(int i = 0; i < length; i++) buffer[i] = payload[i];
  buffer[length] = '\0';
  INFO("[%s] %s\n", topic, buffer);
  ez.canvas.printf("%s = ", topic);
  ez.canvas.println(buffer);
  free(buffer);
}


void setup() {
  m5sys.begin("MQTTMonitor", NETWORK_CONNECTION_UI);  

  // This will have to move to loop() once we can pick a broker
  client.setServer(broker_ip, broker_port);
  client.setCallback(callback);
  INFO("MQTT initialized\n");
}


void loop() {
  VERBOSE("loop()\n");
  ez.canvas.clear();
  ez.canvas.font(&FreeSans9pt7b);
  ez.canvas.scroll(true);
  ez.buttons.show("up # Home # Change ## down #");
  ez.header.show("MQTT Monitor");

  while(true) {
    if(!WiFi.isConnected()) {
      ez.canvas.print("Connecting to network ");
      while(!WiFi.isConnected()) {
        ez.yield();
        delay(500);
        ez.canvas.print(".");
        if(check_for_exit()) {
          WiFi.disconnect();
          client.disconnect();
          m5sys.start_wifi(NETWORK_CONNECTION_UI);
          return;
        }
      }
      ez.canvas.println();
      ez.canvas.print("Connected to ");
      ez.canvas.println(WiFi.SSID());
      ez.canvas.println("Connecting to MQTT broker");
    }

    if(connect_to_mqtt()) {
      client.loop();
    }
    for(int wait = 0; wait < 100; wait++) {
      if(check_for_exit()) {
        WiFi.disconnect();
        client.disconnect();
        m5sys.start_wifi(NETWORK_CONNECTION_UI);
        return;
      }
      ez.canvas.loop();
      ez.yield();
      delay(10);
    }
  }
}
