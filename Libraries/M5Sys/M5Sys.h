#pragma once

#include <M5ez.h>
#include <M5SysBase.h>
#include <Locations.h>

#define NETWORK_CONNECTION_UI   "UI-Pick"


class M5Sys : public M5SysBase {
  public:
    virtual void  begin(const char* appName, const char* connection); // Replaces M5ez.begin
    void          start_wifi(const char* connection);
    bool          wait_for_wifi(int timeout_ms = 15000);
    void          menu();
    Locations     position;
  protected:
    void          disconnect();
    void          auto_connect();
    String        pick_connection(const char* currPick);
};
