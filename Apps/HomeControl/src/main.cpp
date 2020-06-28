#include <M5Sys.h>
#include <PubSubClient.h>
#include <MQTTCon.h>
#include "HomeControl.h"

// This program connects to my private home MQTT control system.
// A flow of messages looks like:
// [sensors/Living/temp] 71.2
// [status/Piano Room/Rooster] 1
// ...

// Perhaps this should move to M5SysBase and modify begin() call:
struct Prog_Info {
  String  prog_name;
  String  connection;
};

void callback(char* topic, byte* payload, unsigned int length); // forward reference
M5Sys         m5sys;
MQTTConRec    conrec      = { "M5SysHomeClient", "#", IPAddress(192, 168, 254, 14), 1883, callback };
Prog_Info     prog_info   = { "HomeControl", NETWORK_CONNECTION_AUTO};
MQTTCon       connection;
HomeControl   home;


bool ascendingNameCaseSensitive(const char* s1, const char* s2) { return 0 > strcmp(s1, s2); }
void canvas_idle() { ez.canvas.print("."); }


// PubSub MQTT callback function, called on receipt of message
//
void callback(char* topic, byte* payload, unsigned int length) {
  VERBOSE("callback(%s, payload, %d)\n", topic, length);
  home.process_mqtt_message(topic, payload, length);
}


// Display the connection progress on the canvas, before displaying menu
//
void start_connection() {
  VERBOSE("start_connection())\n");
  connection.begin(&conrec);
  ez.screen.clear();
  ez.canvas.font(&FreeMono9pt7b);
  ez.canvas.printf("Connecting to %s", prog_info.connection.c_str());
  while(!WiFi.isConnected()) {
    canvas_idle();
    delay(500);
  }
  ez.canvas.println();
  ez.canvas.println("Connected to WiFi");
  ez.canvas.print("Connecting to MQTT");
  connection.connect(canvas_idle);
  ez.canvas.println();
  ez.canvas.println("Connected to MQTT");
}


uint16_t polling_event() {
  VERBOSE("polling_event\n");
  connection.loop();          // let MQTT get busy; callbacks happen in here
  return 1000;                // run every second
}


uint16_t purging_event() {
  VERBOSE("purging_event\n");
  int count = home.purge_clusters_older_than(180);  // delete clusters older than 3 minutes
  if(count) ez.redraw();                            // update screen on change
  return 1000 * 60;                                 // run every minute
}


// Display sensor or controller items
// If isController is true, interpret 0 an 1 values
// as Off and On
//
void display_cluster(Cluster* cluster, bool isController) {
  VERBOSE("display_cluster(%s)\n", cluster->get_name());
  ezMenu m(cluster->get_name());
  m.txtSmall();
  m.buttons("up # Back # select ## down #");
  m.setSortFunction(ascendingNameCaseSensitive);

  int index = 0;
  while(Item* item = cluster->get_item(index++)) {
    String str(item->get_name());
    str.concat('\t');
    if(isController) {
      const char* val = item->get_value();
      str.concat('0' == val[0] ? "Off" : "On");
    }
    else {
      str.concat(item->get_value());
    }
    m.addItem(str);
  }
  while(m.runOnce()) {
    if(isController) {
      String item = ez.leftOf(m.pickName(), "\t");
      DEBUG("selected item = %s\n", item.c_str());
      String topic("ctrl/");
      topic.concat(cluster->get_name());
      topic.concat('/');
      topic.concat(item);

      // toggle the value
      Item* ip = cluster->get_item(item.c_str());
      const char* val = ip->get_value();
      String message('0' == val[0] ? "1" : "0");
      INFO("publishing command: topic = '%s', message = '%s'\n", topic.c_str(), message.c_str());
      connection.publish(topic.c_str(), message.c_str());
      // Update the UI
      item.concat('\t');
      item.concat('0' == val[0] ? "On" : "Off"); // Note inverted logic
      m.setCaption(m.pickName(), item);
    }
  }
}


// Display all sensor clusters collected so far
//
void display_sensors() {
  VERBOSE("display_sensors()\n");
  ezMenu m("Home Sensors");
  m.txtSmall();
  m.buttons("up # Back # select ## down #");
  m.setSortFunction(ascendingNameCaseSensitive);

  int index = 0;
  while(Cluster* p = home.get_sensor(index++)) {
    m.addItem(p->get_name());
  }
  while(m.runOnce()) {
    Cluster* cluster = home.get_sensor(m.pickName().c_str());
    if(cluster) display_cluster(cluster, false);
  }
}


// Display all controller clusters collected so far
//
void display_controllers() {
  VERBOSE("display_controllers()\n");
  ezMenu m("Home Controllers");
  m.txtSmall();
  m.buttons("up # Back # select ## down #");
  m.setSortFunction(ascendingNameCaseSensitive);

  int index = 0;
  while(Cluster* p = home.get_controller(index++)) {
    m.addItem(p->get_name());
  }
  while(m.runOnce()) {
    Cluster* cluster = home.get_controller(m.pickName().c_str());
    if(cluster) display_cluster(cluster, true);
  }
}


void setup() {
  m5sys.begin(prog_info.prog_name.c_str(), prog_info.connection.c_str());
  start_connection();
}


void loop() {
  VERBOSE("loop()\n");
  ezMenu m("Home Control");
  m.txtSmall();
  m.buttons("up # back | Home # select ## down #");
  ez.addEvent(polling_event, 500);            // half a second out of step with purging
  ez.addEvent(purging_event, 1000 * 60 * 3);  // wait 3 minutes to run first purge
  m.addItem("Sensors", display_sensors);
  m.addItem("Controllers", display_controllers);
  m.run();                                    // exits when 'back | Home' is selected
  m5sys.goHome();
}
