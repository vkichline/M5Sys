#pragma once

#include "Arduino.h"
#include "FS.h"
#include "SD_MMC.h"

#define BUF_SIZE 24 * 1024
#define SNAP_LEN 2324     // max len of each received packet

extern bool useSD;

class Buffer {
  public:
    Buffer();
    void open(fs::FS* fs);
    void close(fs::FS* fs);
    void addPacket(uint8_t* buf, uint32_t len);
    void save(fs::FS* fs);
    void forceSave(fs::FS* fs);
  private:
    void write(int32_t  n);
    void write(uint32_t n);
    void write(uint16_t n);
    void write(uint8_t* buf, uint32_t len);
    
    uint8_t* bufA;
    uint8_t* bufB;

    uint32_t bufSizeA = 0;
    uint32_t bufSizeB = 0;

    bool writing  = false;   // accepting writes to buffer
    bool useA     = true;    // writing to bufA or bufB
    bool saving   = false;   // currently saving onto the SD card

    String fileName = "/0.pcap";
    File file;
};
