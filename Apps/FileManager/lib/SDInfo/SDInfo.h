#pragma once

#include <SD.h>
#include <M5Sys.h>

#define INFO_TABSTOP   164
#define INFO_TOP_GAP   40
#define INFO_L_MARGIN  16


struct SD_Info {
  sdcard_type_t cardType;  // None = 0, MMC, SD, SDHC, Unknown
  uint64_t      cardSize;
  uint64_t      totalBytes;
  uint64_t      usedBytes;
  int           numSectors;
  int           sectorSize;
};


class SDInfo {
  public:
    void  get_info(SD_Info& info);
    void  dump_info(SD_Info& info);
  
  private:
    void  info_to_canvas(const char* label, uint64_t val, bool newLine = true);
    void  info_to_canvas(const char* label, int val, bool newLine = true);
    void  info_to_canvas(const char* label, String val, bool newLine = true);
};
