#include <M5Sys.h>
#include <FileScanner.h>
#include <SDCleaner.h>
#include <SDInfo.h>

M5Sys   m5sys;
bool    spiffs_disabled;


void show_sd_info() {
  VERBOSE("get_sd_info()\n");
  ez.canvas.lmargin(INFO_L_MARGIN);
  ez.canvas.clear();
  ez.header.show("SD Card Info");
  ez.canvas.font(&FreeSans12pt7b);
  ez.canvas.y(INFO_TOP_GAP);

  SDInfo  sd_info;
  SD_Info info;
  sd_info.get_info(info);
  sd_info.dump_info(info);

  ez.buttons.wait("Back");
  ez.header.clear();
}


void show_spiffs_info() {
  VERBOSE("get_spiffs_info()\n");
  ez.canvas.lmargin(30);
  ez.canvas.clear();
  ez.header.show("SPIFFS Info");
  ez.canvas.font(&FreeSans12pt7b);
  ez.canvas.y(INFO_TOP_GAP);

  ez.canvas.print("Total Bytes");
  ez.canvas.x(200);
  ez.canvas.println(SPIFFS.totalBytes());
  ez.canvas.print("Used Bytes");
  ez.canvas.x(200);
  ez.canvas.println(SPIFFS.usedBytes());

  ez.buttons.wait("Back");
  ez.header.clear();
}


void format_spiffs() {
  VERBOSE("format_spiffs()\n");
  String result = ez.msgBox("Format SPIFFS", "Format SPIFFS:\nAre you sure?", "Cancel ## OK");
  DEBUG("result = %s\n", result);
  if(0 == result.compareTo("OK")) {
    INFO("Formatting SPIFFS file system\n");
    bool fmtResult = SPIFFS.format();
    DEBUG("Format results = %d\n", fmtResult);
    ez.msgBox("Format SPIFFS", fmtResult ? "Format succeeded.\n" : "Format failed.\n", "OK");
  }
}


void scan_sd_files() {
  FileScanner scanner(SD);
  scanner.menu("SD Card Files");
}


void scan_spiffs_files() {
  FileScanner scanner(SPIFFS);
  scanner.menu("SPIFFS Files");
}


void remove_hidden_files() {
  SDCleaner cleaner;
  cleaner.remove_hidden_files();
}


void setup() {
  m5sys.begin("FileMgr", NETWORK_CONNECTION_NONE);
  spiffs_disabled = !m5sys.start_spiffs();
}


void loop() {
  VERBOSE("Loop()\n");  // Normally a bad idea, but this program only loops after button presses
  ezMenu m("File Manager");
  m.txtSmall();
  m.buttons("up # back | Home # select ## down #");
  m.addItem("SD Card Info",               show_sd_info);
  m.addItem("Display SD Card Files",      scan_sd_files);
  m.addItem("Remove All Hidden SD Files", remove_hidden_files);
  if(!spiffs_disabled) {
    m.addItem("SPIFFS Info",              show_spiffs_info);
    m.addItem("Display SPIFFS Files",     scan_spiffs_files);
  }
  m.addItem("Format SPIFFS",            format_spiffs);
  if(0 == m.runOnce()) m5sys.goHome();
}
