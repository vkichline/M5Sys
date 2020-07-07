#pragma once

#include <M5Stack.h>
#include <Logger.h>

#define NETWORK_CONNECTION_NONE "None"
#define NETWORK_CONNECTION_AUTO "Autoconnect"


class M5SysBase {
  public:
    virtual void  begin(const char* appName, const char* connection);
    virtual bool  wait_for_wifi(int timeout_ms = 15000);
    void          start_wifi(const char* connection);
    bool          start_spiffs(bool formatOnFailure = false);
    void          goHome();                     // Load the main menu program and restart
    void          launch(const char* progName); // May include or omit leading '/' and trailing '.bin'
    void          wait_for_any_button(bool wait_for_clear = false);
    Logger        log;
  protected:
    void          startup_test();               // Test the buttons at begin() time
    void          disconnect();
    void          auto_connect();
    bool          connect_to_ssid(const char* currPick);
};
