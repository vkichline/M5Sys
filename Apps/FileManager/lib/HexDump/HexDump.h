#pragma once
#include <Arduino.h>

class HexDump {
  public:
    HexDump(File file);
    int run();  // return an error number
  
  protected:
    File    file;
    int     offset;
    void    dump_row(const char* data, int offset, int address = 0, int limit = 8);
    void    dump_hex(int value, int digits, const char* followed_by);
    char*   reverse(char* str);
};