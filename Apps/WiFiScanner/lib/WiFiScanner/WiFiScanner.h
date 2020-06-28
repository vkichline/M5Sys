#pragma once

// WiFiScanner for M5Sys project

#include <M5Sys.h>

#define WIFISCANNER_BUFFER_SIZE   256
#define WIFISCANNER_INDENT        110

struct SSID_Node {
  String      name;
  String      kind;
  String      bssid;
  int         channel;
  int         rssi;
  SSID_Node*  next;
};

typedef SSID_Node*  SSID_NodePtr;


class WiFiScanner {
  public:
                  WiFiScanner();
                  ~WiFiScanner();
    void          begin();
    int           scan(ezMenu& m);
    void          clear();  // deletes linked list
    void          display_node(const char* name);
    SSID_NodePtr  get_node(const char* name);

  protected:
    bool          started;
    char*         storage;  // Saves first half of a formatted line for second half to print.
    void          print_label(const char* label);
    void          print_value(const char* format, ...);
    SSID_NodePtr  ssid_list;
    SSID_NodePtr  ssid_last;
    void          add_to_list(String name, int kind, const uint8_t *bssid, int channel, int rssi);
    void          free_list();
};
