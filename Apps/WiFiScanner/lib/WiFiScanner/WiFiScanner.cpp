#include <WiFi.h>
#include "WiFiScanner.h"

extern M5Sys   m5sys;


WiFiScanner::WiFiScanner() {
  VERBOSE("WiFiScanner::WiFiScanner()\n");
  storage   = (char*)malloc(WIFISCANNER_BUFFER_SIZE);
  started   = false;
  ssid_list = nullptr;
  ssid_last = nullptr;
}


WiFiScanner::~WiFiScanner() {
  free(storage);
  free_list();
}


void WiFiScanner::begin() {
  VERBOSE("WiFiScanner::begin()\n");
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  started = true;
}


// Build a menu list of WiFi ssid names, and create a linked list of
// nodes all available data of each ssid for display later as details.
// Return the number of networks found.
//
int WiFiScanner::scan(ezMenu& m) {
  VERBOSE("WiFiScanner::scan(menu)\n");
  if(!started) {
    ERROR("WiFiScanner::scan: called without a preceding call to begin()\n");;
    return false;
  }

  // Clear the menu and linked list before appending to it:
  while(m.deleteItem(1)) {}
  free_list();

  int hiddenNetworkNumber = 1;
  int nNetworks = WiFi.scanNetworks(false, true);
  INFO("%d networks found.\n", nNetworks);
  for(int i = 0; i < nNetworks; i++) {
    String str(WiFi.SSID(i)); // str is the name of a network
    if(0 == str.length()) {
      str = "Hidden network #";
      str.concat(hiddenNetworkNumber++);
      DEBUG("providing hidden network name: %s\n", str.c_str());
    }
    add_to_list(str, WiFi.encryptionType(i), WiFi.BSSID(i), WiFi.channel(i), WiFi.RSSI(i));
    str.concat('\t');
    str.concat(WiFi.channel(i));
    m.addItem(str);
  }
  // enumerate the linked list for debugging
  if(m5sys.log.get_level() >= LOG_DEBUG) {
    int count = 0;
    SSID_NodePtr nodes = ssid_list;
    DEBUG("Linked list of ssid nodes:\n");
    while(nodes) {
      DEBUG("  Node %d: %s, %s, %d, %d\n", count, nodes->name.c_str(), nodes->kind.c_str(), nodes->rssi, nodes->channel);
      count++;
      nodes = nodes->next;
    }
    if(count != nNetworks) {
      ERROR("WiFiScanner::scan: number of nodes does not equal number of nextworks. Nodes = %d, Networks = %d\n", count, nNetworks);
    }
  }
  return nNetworks;
}


void WiFiScanner::add_to_list(String name, int kind, const uint8_t *bssid, int channel, int rssi) {
  VERBOSE("WiFiScanner::insert_node(%s, %d, %d, %d, %d)\n", name.c_str(), kind, bssid, channel, rssi);
  SSID_Node* node = new(SSID_Node);
  sprintf(storage, "%02x:%02x:%02x:%02x:%02x:%02x", bssid[0], bssid[1], bssid[2], bssid[3], bssid[4], bssid[5]);

  node->name    = name;
  node->bssid   = String(storage);
  node->channel = channel;
  node->rssi    = rssi;
  node->next    = nullptr;
  switch(kind) {
    case WIFI_AUTH_OPEN:            node->kind = "Open";      break;
    case WIFI_AUTH_WEP:             node->kind = "WEP";       break;
    case WIFI_AUTH_WPA_PSK:         node->kind = "WPA";       break;
    case WIFI_AUTH_WPA2_PSK:        node->kind = "WPA2";      break;
    case WIFI_AUTH_WPA_WPA2_PSK:    node->kind = "WPA/WPA2";  break;
    case WIFI_AUTH_WPA2_ENTERPRISE: node->kind = "Ent";       break;
    default: {
      node->kind = "Unknown(";
      node->kind.concat(kind);
      node->kind.concat(")");
      break;
    }
  }
  DEBUG("node->kind = %s\n", node->kind.c_str());
  INFO("%s, %s, %s, %d, %d\n", node->name.c_str(), node->kind.c_str(), node->bssid.c_str(), node->channel, node->rssi);

  // Insert node into linked list
  if(!ssid_last) {
    ssid_list = node;
    ssid_last = node;
    DEBUG("inserted node at beginning of list\n");
  }
  else {
    ssid_last->next = node;
    ssid_last       = node;
    DEBUG("inserted node at end of list\n");
  }
}


void WiFiScanner::free_list() {
  VERBOSE("free_list()\n");
  int count = 0;
  while(ssid_list) {
    SSID_NodePtr  nxt = ssid_list->next;
    // BUGBUG Do the strings leak? Do I need to delete them?
    DEBUG("Deleting node %d of ssid_list: %s\n", count, ssid_list->name.c_str());
    if(nullptr == ssid_list->next) {
      if(ssid_last != ssid_list) {
        DEBUG("WiFiScanner::free_list(): Unexpected condition: last instance of ssid_list does not match ssid_last\n");
      }
    }
    delete(ssid_list);
    ssid_list = nxt;
    count++;
  }
  DEBUG("%d nodes deleted\n", count);
  ssid_list = nullptr;
  ssid_last = nullptr;
}


// Return the node whose name matches the name provided,
// or nullptr if not found.
//
SSID_NodePtr WiFiScanner::get_node(const char* name) {
  VERBOSE("WiFiScanner::get_node(%s)\n", name);
  SSID_NodePtr  node = ssid_list;
  while(node) {
    DEBUG("WiFiScanner::get_node: node->name = %s\n", node->name.c_str());
    if(node->name == name) {
      DEBUG("Match found.\n");
      return node;
    }
    node = node->next;
  }
  DEBUG("WiFiScanner::get_node: no matching node found\n");
  return nullptr;
}


void WiFiScanner::display_node(const char* name) {
  VERBOSE("WiFiScanner::display_node(%s)\n", name);
  strncpy(storage, name, WIFISCANNER_BUFFER_SIZE); // can't modify the const char*
  char* pos = strchr(storage, '\t');
  if(pos) {
    DEBUG("WiFiScanner::display_node: name contains a tab. Truncating.\n");
    *pos = '\0';
    DEBUG("truncated name = %s\n", storage);
  }
  SSID_NodePtr  node = get_node(storage);
  if(!node) {
    DEBUG("No matching node\n");
    return;
  }
  ez.screen.clear();
  ez.header.show("SSID Info");
  ez.buttons.show("# Home # down");
  ez.canvas.font(&FreeSans9pt7b);
  ez.canvas.lmargin(10);
  ez.canvas.println("");

  print_label("SSID");      print_value("%s", node->name.c_str());
  print_label("Security");  print_value("%s", node->kind.c_str());
  print_label("BSSID");     print_value("%s", node->bssid.c_str());
  print_label("Channel");   print_value("%d", node->channel);
  print_label("RSSI");      print_value("%d", node->rssi);
  if(0 == node->kind.compareTo("Open")) {
    String result = ez.buttons.wait("# OK # Add Auto");
    if(0 == result.compareTo("Add Auto")) {
      if(-1 == ez.wifi.indexForSSID(node->name)) {
        INFO("Adding SSID %s to autoconnects\n", node->name.c_str());
        ez.wifi.add(node->name, "");
        ez.wifi.writeFlash();
      }
      else {
        DEBUG("SSID %s is already in autoconnects\n", node->name.c_str());
      }
    }
  }
  else {
    ez.buttons.wait("OK");
  }
}


void WiFiScanner::print_label(const char* label) {
  ez.canvas.print(label);
  ez.canvas.x(WIFISCANNER_INDENT);
  snprintf(storage, WIFISCANNER_BUFFER_SIZE, "%s\t", label);
}


void WiFiScanner::print_value(const char* format, ...) {
  char buffer[WIFISCANNER_BUFFER_SIZE];
  va_list argp;
  va_start(argp, format);
  vsnprintf(buffer, WIFISCANNER_BUFFER_SIZE, format, argp);
  va_end(argp);
  ez.canvas.println(buffer);
  DEBUG("%s%s\n", storage, buffer);
}
