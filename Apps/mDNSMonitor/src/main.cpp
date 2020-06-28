#include <M5Sys.h>
#include <WiFi.h>
#include <ESPmDNS.h>

#define     CONNECTION_DELAY  100
#define     POLLING_DELAY     1000

M5Sys       m5sys;
const char* services[]        = { "http", "arduino", "workstation", "smb", "afpovertcp", "ftp", "ipp", "printer" };
int         num_services      = sizeof(services) / sizeof(char*);
int         curr_service      = 0;
bool        responder_started = false;
long        last_poll         = 0;

std::vector<String> itemVector;

void browseService(const char* service, const char* protocol){
  VERBOSE("browseService(%s, %s)\n", service, protocol);
  ez.header.title(String("Browsing service: ") + service);
  DEBUG("Browsing for service _%s._%s.local.\n", service, protocol);
  int n = MDNS.queryService(service, protocol);
  if(n == 0) {
    DEBUG("no %s/%s services found\n", service, protocol);
  }
  else {
    for (int i = 0; i < n; ++i) {
      INFO("%s: \"%s\" %s:%d\n", service, MDNS.hostname(i).c_str(), MDNS.IP(i).toString().c_str(), MDNS.port(i));
      // Item will be int the form: full data | display data
      String item(service);
      item.concat(", ");
      item.concat(MDNS.hostname(i));
      item.concat(", ");
      item.concat(MDNS.IP(i).toString());
      item.concat(':');
      item.concat(MDNS.port(i));
      if(std::find(itemVector.begin(), itemVector.end(), item) == itemVector.end()) {
        itemVector.push_back(item);
        //ez.canvas.println(item);
        ez.canvas.print(service);
        ez.canvas.x(104);
        ez.canvas.println(MDNS.hostname(i));
        ez.canvas.x(104);
        ez.canvas.print(MDNS.IP(i).toString());
        ez.canvas.print(':');
        ez.canvas.println(MDNS.port(i));
      }
    }
  }
}


// Called every 100 mS until the network is connected.
// When connected, remove menu item #1
// If the mDNS browser fails to immediately start, add a new error menu item.
//
uint16_t connect_event() {
  //VERBOSE("network_connect_event()\n");  happens too much to log
  if(WiFi.status() == WL_CONNECTED) {
    DEBUG("Connected to network %s, ip = %s\n", WiFi.SSID().c_str(), WiFi.localIP().toString().c_str());
    ez.canvas.println("Connected\n");

    if (MDNS.begin("ESP32_Browser")) {
      DEBUG("Responder started\n");
      responder_started = true;
    }
    else {
      ERROR("Error setting up MDNS responder\n");
      ez.canvas.println("Failed to start mDNS responder");
    }
    return 0; // never call back
  }
  return CONNECTION_DELAY; // call back in 100 mS
}


// Called once a second.
// Cycle through the services and add menu items representing unique onces.
//
uint16_t polling_event() {
  if(!last_poll) {
    // if the global last_poll is uninitialized, set it to 'now'
    last_poll = millis();
  }
  else {
    if(millis() < last_poll + POLLING_DELAY) {
      DEBUG("why was the event called so fast?\n");   // HACKHACK
      return POLLING_DELAY;
    }
    last_poll = millis();
  }
  
  if(responder_started) {
    const char* svc = services[curr_service++];
    DEBUG("selecting service #%d: %s\n", curr_service, svc);
    if(curr_service >= num_services) curr_service = 0;
    browseService(svc, "tcp");
  }
  else {
    VERBOSE("Responder not yet started\n");
  }
  return POLLING_DELAY;
}


void setup() { m5sys.begin("mDNSMonitor", NETWORK_CONNECTION_UI); }

void loop() {
  VERBOSE("loop()\n");
  ez.header.show("mDNS Monitor");
  ez.canvas.scroll(true);
  ez.canvas.lmargin(8);
  ez.canvas.font(&FreeSans9pt7b);
  ez.buttons.show("up # Home # Stop ## down #");
  ez.canvas.println("Connecting to network");

  ez.addEvent(connect_event);
  ez.addEvent(polling_event, 10);
  while(true) {
    String result = ez.buttons.wait();
    INFO("User selected %s\n", result.c_str());
    if(0 == result.compareTo("Home")) m5sys.goHome();
    else if(0 == result.compareTo("Stop")) {
      ez.header.title("Browsing Paused...");
      ez.buttons.show("up # Home # Start ## down #");
      ez.removeEvent(connect_event);
      ez.removeEvent(polling_event);
    }
    else if(0 == result.compareTo("Start")) {
      ez.buttons.show("up # Home # Stop ## down #");
      ez.addEvent(connect_event);
      ez.addEvent(polling_event, 10);
    }
    else if(0 == result.compareTo("up") || 0 == result.compareTo("down")) {
      ez.canvas.loop();
    }
  }
}
