#include <SPIFFS.h>
#include <M5Sys.h>
#include "SysInfo.h"

typedef void (SysInfo::*page_handler)();

extern  M5Sys   m5sys;


SysInfo::SysInfo() {
  VERBOSE("SysInfo::SysInfo()\n");
    storage   = (char*)malloc(SYSINFO_BUFFER_SIZE);
    num_pages = 3;
    indent    = 0;
    page      = 0;
}


SysInfo::~SysInfo() {
  VERBOSE("SysInfo::~SysInfo()\n");
  free(storage);
}


void SysInfo::dispatch() {
  VERBOSE("SysInfo::dispatch(): page = %d\n", page);
  switch(page) {
    case 0: page1();  break;
    case 1: page2();  break;
    case 2: page3();  break;
    default:
      page = 0;
      page1();
      break;
  }
}


void SysInfo::display_info() {
  VERBOSE("SysInfo::display_info()\n");
  Wire.begin();
  page1();
  while(true) {
    String btn = ez.buttons.poll();
    if(btn == "up") {
      DEBUG("selection = up\n");
      if(0 > --page) page = num_pages - 1;
      dispatch();
    }
    else if (btn == "down") {
      DEBUG("selection = %s\n", btn);
      if(++page >= num_pages) page = 0;
      dispatch();
    }
    else if (btn == "Back") {
      DEBUG("selection = %s\n", btn);
      break;
    }
  }
}


void SysInfo::page1() {
  VERBOSE("SysInfo::page1()\n");
  indent = SYSINFO_PAGE_1_INDENT;    // Used by print_label()

  ez.canvas.clear();
  ez.header.show("System Info  (1/3)");
  ez.buttons.show("# Back # down");
  ez.canvas.font(&FreeSans9pt7b);
  ez.canvas.lmargin(10);
  ez.canvas.println("");

  print_label("CPU freq:");     print_value("%d MHz", ESP.getCpuFreqMHz());
  print_label("CPU cores:");    print_value("2"); // BUGBUG!!
  print_label("Chip rev.:");    print_value("%d", String(ESP.getChipRevision()));
  print_label("Flash speed:");  print_value("%d MHz", ESP.getFlashChipSpeed() / 1049600);
  print_label("Flash size:");   print_value("%d MB", ESP.getFlashChipSize() / 1049600);
  print_label("ESP SDK:");      print_value("%s", ESP.getSdkVersion());
  print_label("M5ez:");         print_value("%s", ez.version().c_str());
}


void SysInfo::page2() {
  VERBOSE("SysInfo::page2()\n");
  const char*   SD_Type[5]  = { "None", "MMC", "SD", "SDHC", "Unknown" };
  const int     sd_type     = SD.cardType();

  indent = SYSINFO_PAGE_2_INDENT;    // Used by print_label()
  ez.canvas.clear();
  ez.header.show("System Info  (2/3)");
  ez.buttons.show("up # Back # down");
  ez.canvas.font(&FreeSans9pt7b);
  ez.canvas.lmargin(10);
  ez.canvas.println("");

  SPIFFS.begin();
  print_label("Free RAM:");       print_value("%d bytes", ESP.getFreeHeap());
  print_label("Min free seen:");  print_value("%d bytes", esp_get_minimum_free_heap_size());
  print_label("SPIFFS size:");    print_value("%d bytes", SPIFFS.totalBytes());
  print_label("SPIFFS used:");    print_value("%d bytes", SPIFFS.usedBytes());
  print_label("SD type:");        print_value("%s", SD_Type[sd_type]);
  if (sd_type != 0) {
    print_label("SD size:");      print_value("%ld MB", SD.cardSize()   / 1048576);
    print_label("SD used:");      print_value("%ld MB", SD.usedBytes()  / 1048576);
  }
  SPIFFS.end();
}


void SysInfo::page3() {
  VERBOSE("SysInfo::page3()\n");
  static char hex[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
  ez.canvas.lmargin(35);
  ez.canvas.font(&FreeSans9pt7b);
  ez.header.show("System Info  (3/3)");
  ez.buttons.show("up # Back #");

  ez.canvas.clear();
  ez.canvas.y(40);
  ez.canvas.x(100);
  ez.canvas.println("I2C Bus Scan");
  ez.canvas.font(&FreeMono9pt7b);
  ez.canvas.y(66);
  for(int major = 0; major < 8; major++) {
    ez.canvas.printf("%dX  ", major);
    for(int minor = 0; minor < 16; minor++) {
      int addr = major * 16 + minor;
      if(0 == addr || addr >= 127) {
        ez.canvas.print(" ");
      }
      else {
        Wire.beginTransmission(addr);
        uint8_t error = Wire.endTransmission();
        if(0 == error) {
          ez.canvas.printf("%c", hex[minor]);
          INFO("Device found at %c%c\n", hex[major], hex[minor]);
        }
        else {
          ez.canvas.print("-");
        }
      }
    }
    ez.canvas.println();
  }
}


void SysInfo::print_label(const char* label) {
  ez.canvas.print(label);
  ez.canvas.x(indent);
  snprintf(storage, SYSINFO_BUFFER_SIZE, "%s\t", label);
}


void SysInfo::print_value(const char* format, ...) {
  char buffer[SYSINFO_BUFFER_SIZE];
  va_list argp;
  va_start(argp, format);
  vsnprintf(buffer, SYSINFO_BUFFER_SIZE, format, argp);
  va_end(argp);
  ez.canvas.println(buffer);
  INFO("%s%s\n", storage, buffer);
}
