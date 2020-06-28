#include "SDInfo.h"
#include <sd_diskio.h>

extern M5Sys  m5sys;    // Needed by logging macros


void SDInfo::get_info(SD_Info& info) {
  VERBOSE("SDInfo::get_info(info)\n");
  info.cardType    = SD.cardType();  // None = 0, MMC, SD, SDHC, Unknown
  info.cardSize    = SD.cardSize();
  info.totalBytes  = SD.totalBytes();
  info.usedBytes   = SD.usedBytes();
  info.numSectors  = sdcard_num_sectors(0);
  info.sectorSize  = sdcard_sector_size(0);
}


// Display info both to log output and to the canvas
void SDInfo::dump_info(SD_Info& info) {
  VERBOSE("SDInfo::dump_info(info)\n");
  String cardTypeName;

  switch(info.cardType) {
    case CARD_NONE:     cardTypeName = "None";    break;
    case CARD_MMC:      cardTypeName = "MMC";     break;
    case CARD_SD:       cardTypeName = "SD";      break;
    case CARD_SDHC:     cardTypeName = "SDHC";    break;
    case CARD_UNKNOWN:  cardTypeName = "Unknown"; break;
  }

  INFO("Card Type:   %s\n",    cardTypeName.c_str());
  INFO("Card Size:   %lld\n",  info.cardSize);
  INFO("Total Bytes: %lld\n",  info.totalBytes);
  INFO("Used Bytes:  %lld\n",  info.usedBytes);
  INFO("Num Sectors: %d\n",    info.numSectors);
  INFO("Sector Size: %d\n",    info.sectorSize);

  info_to_canvas("Card Type",   cardTypeName);
  info_to_canvas("Card Size",   info.cardSize);
  info_to_canvas("Total Bytes", info.totalBytes);
  info_to_canvas("Used Bytes",  info.usedBytes);
  info_to_canvas("Num Sectors", info.numSectors);
  info_to_canvas("Sector Size", info.sectorSize);
}


void  SDInfo::info_to_canvas(const char* label, uint64_t val, bool newLine) {
  VERBOSE("SDInfo::info_to_canvas(%s, %lld, %s)\n", label, val, newLine ? "true" : " false");
  ez.canvas.print(label);
  ez.canvas.x(INFO_TABSTOP);
  ez.canvas.printf("%lld", val);
  if(newLine) ez.canvas.println();
}


void  SDInfo::info_to_canvas(const char* label, int val, bool newLine) {
  VERBOSE("SDInfo::info_to_canvas(%s, %d, %s)\n", label, val, newLine ? "true" : " false");
  ez.canvas.print(label);
  ez.canvas.x(INFO_TABSTOP);
  ez.canvas.printf("%d", val);
  if(newLine) ez.canvas.println();
}


void  SDInfo::info_to_canvas(const char* label, String val, bool newLine) {
  VERBOSE("SDInfo::info_to_canvas(%s, %s, %s)\n", label, val, newLine ? "true" : " false");
  ez.canvas.print(label);
  ez.canvas.x(INFO_TABSTOP);
  ez.canvas.printf("%s", val.c_str());
  if(newLine) ez.canvas.println();
}
