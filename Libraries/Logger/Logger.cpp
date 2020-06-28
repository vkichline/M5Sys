#include <Preferences.h>
#include <M5ez.h>
#include "Logger.h"

#define M5SYS_PREFS_NAME      "M5Sys"
#define _LOGGER_BUFFER_SIZE   256

String          Logger::program_name;
Print*          Logger::log_device;
File            Logger::log_file;
m5SysLogLevel_t Logger::log_level;


void Logger::begin(const char* appName) {
  Logger::program_name  = appName;
  Logger::log_device    = &Serial;
  Logger::log_level     = LOG_NONE;
  set_from_prefs();
  verbose("M5SysBase::begin()\n");
  info("starting program %s\n", appName);
}


m5SysLogLevel_t Logger::get_level() { 
  return Logger::log_level;
}


void Logger::menu() {
  verbose("Logger::menu()\n");
  Preferences prefs;

   while(true) {
    bool stat = prefs.begin(M5SYS_PREFS_NAME, true); // Open in R/W mode
    if(!stat) { error("Logger::menu: failed to open %s\n", M5SYS_PREFS_NAME); }
    m5SysLogLevel_t       l_level   = (m5SysLogLevel_t)prefs.getInt(     "log_level",  LOG_INFO);
    m5sysLogOutputKind_t  l_kind    = (m5sysLogOutputKind_t)prefs.getInt("log_kind",   LOG_OUTPUT_SERIAL);
    bool                  l_append  = prefs.getBool(                     "log_append", false);
    prefs.end();
    debug("Logger::menu(): log_level from preference was %d\n", l_level);

    ezMenu m("Logging Settings");
    m.txtSmall();
    m.buttons("up # Back # select ## down #");

    String level_item("Set Level");
    switch(l_level) {
      case LOG_NONE:    level_item.concat("\tNone");    break;
      case LOG_ERROR:   level_item.concat("\tError");   break;
      case LOG_INFO:    level_item.concat("\tInfo");    break;
      case LOG_DEBUG:   level_item.concat("\tDebug");   break;
      case LOG_VERBOSE: level_item.concat("\tVerbose"); break;
    }

    String target_item("Set Target");
    switch(l_kind) {
      case LOG_OUTPUT_SERIAL:   target_item.concat("\tSerial");   break;
      case LOG_OUTPUT_SERIAL2:  target_item.concat("\tSerial2");  break;
      case LOG_OUTPUT_SD_FILE:  target_item.concat(l_append ? "\tFile (a)" : "\tFile (w)");
    }

    m.addItem(level_item,   menu_logging_level);
    m.addItem(target_item,  menu_logging_target);
    if(0 == m.runOnce()) return;
  }
}


void Logger::set(const m5SysLogLevel_t level) {
  verbose("Logger::set(%d)\n", level);
  set(level, *Logger::log_device);
}


void Logger::set(Print &device) {
  verbose("Logger::set(Print Device)\n");
  set(Logger::log_level, device);
}


void Logger::set(const m5SysLogLevel_t level, Print &device) {
  Logger::log_level  = level;
  Logger::log_device = &device;
  verbose("Logger::set(%d)\n", level);  // Any way to indicate device?
  String levelName;
  switch(level) {
    case LOG_NONE:    levelName = "None";     break;
    case LOG_ERROR:   levelName = "Error";    break;
    case LOG_INFO:    levelName = "Info";     break;
    case LOG_DEBUG:   levelName = "Debug";    break;
    case LOG_VERBOSE: levelName = "Verbose";  break;
    default:          levelName = "Unknown";  break;
  }
  debug("M5Sys debug level set to: %s\n", levelName.c_str());
}


void Logger::error(const char* format, ...) {
  if(Logger::log_level >= LOG_ERROR) {
    va_list argp;
    va_start(argp, format);
    output_log("[E] ", format, argp);
    va_end(argp);
  }
}


void Logger::info(const char* format, ...) {
  if(Logger::log_level >= LOG_INFO) {
    va_list argp;
    va_start(argp, format);
    output_log("[I] ", format, argp);
    va_end(argp);
  }
}


void Logger::debug(const char* format, ...) {
  if(Logger::log_level >= LOG_DEBUG) {
    va_list argp;
    va_start(argp, format);
    output_log("[D] ", format, argp);
    va_end(argp);
  }
}


void Logger::verbose(const char* format, ...) {
  if(Logger::log_level >= LOG_VERBOSE) {
    va_list argp;
    va_start(argp, format);
    output_log("[V] ", format, argp);
    va_end(argp);
  }
}


void Logger::menu_logging_level() {
  verbose("menu_logging_level()\n");
  Preferences prefs;

  prefs.begin(M5SYS_PREFS_NAME, true);
  m5SysLogLevel_t l_level = (m5SysLogLevel_t)prefs.getInt("log_level", LOG_INFO);
  prefs.end();
  debug("log_level from preference was %d\n", l_level);

  ezMenu m("Logging Level");
  m.txtSmall();
  m.buttons("up # Back # select ## down #");
  m.addItem((LOG_NONE    == l_level) ? "None\tSelected"    : "None"   );
  m.addItem((LOG_ERROR   == l_level) ? "Error\tSelected"   : "Error"  );
  m.addItem((LOG_INFO    == l_level) ? "Info\tSelected"    : "Info"   );
  m.addItem((LOG_DEBUG   == l_level) ? "Debug\tSelected"   : "Debug"  );
  m.addItem((LOG_VERBOSE == l_level) ? "Verbose\tSelected" : "Verbose");
  if(0 == m.runOnce()) return;

  m5SysLogLevel_t new_l_level = l_level;
  String selection = m.pickName();
  debug("Menu selection = '%s'\n", m.pickName().c_str());
  if     (selection.startsWith("None"))    new_l_level = LOG_NONE;
  else if(selection.startsWith("Error"))   new_l_level = LOG_ERROR;
  else if(selection.startsWith("Info"))    new_l_level = LOG_INFO;
  else if(selection.startsWith("Debug"))   new_l_level = LOG_DEBUG;
  else if(selection.startsWith("Verbose")) new_l_level = LOG_VERBOSE;
  
  debug("log_level = %d, new_log_level = %d\n", l_level, new_l_level);
  if(new_l_level != l_level) {
    prefs.begin(M5SYS_PREFS_NAME, false);
    debug("writing log level of %d to prefs\n", new_l_level);
    int sz = prefs.putInt("log_level", new_l_level);
    if(4 != sz) { error("failed to write log_level to prefs\n"); }
    prefs.end();
    set(new_l_level);
  }
}


void Logger::menu_logging_target() {
  verbose("Logger::menu_logging_target()\n");
  Preferences prefs;
  prefs.begin(M5SYS_PREFS_NAME, true);
  m5sysLogOutputKind_t l_kind    = (m5sysLogOutputKind_t)prefs.getInt("log_kind",   LOG_OUTPUT_SERIAL);
  bool                 l_append  = prefs.getBool(                     "log_append", false);
  prefs.end();
  debug("Log kind = %d, log_append = %s\n", l_kind, l_append ? "true" : "false");

  bool sel[] =  { l_kind == LOG_OUTPUT_SERIAL,
                  l_kind == LOG_OUTPUT_SERIAL2,
                  l_kind == LOG_OUTPUT_SD_FILE && !l_append,
                  l_kind == LOG_OUTPUT_SD_FILE && l_append
                };
  ezMenu m("Logging Target");
  m.txtSmall();
  m.buttons("up # Back # select ## down #");
  m.addItem(sel[0] ? "Serial\tSelected"               : "Serial");
  m.addItem(sel[1] ? "Serial2\tSelected"              : "Serial2");
  m.addItem(sel[2] ? "Log File (overwrite)\tSelected" : "Log File (overwrite)");
  m.addItem(sel[3] ? "Log File (append)\tSelected"    : "Log File (append)");
  if(0 == m.runOnce()) return;

  m5sysLogOutputKind_t  new_l_kind    = l_kind;
  bool                  new_l_append  = l_append;
  String selection = m.pickName();
  debug("Menu selection = '%s'\n", m.pickName().c_str());
  if(selection.startsWith("Serial2")) {
    new_l_kind    = LOG_OUTPUT_SERIAL2;
  }
  else if(selection.startsWith("Serial")) {
    new_l_kind    = LOG_OUTPUT_SERIAL;
  }
  else if(selection.startsWith("Log File (overwrite)")) {
    new_l_kind    = LOG_OUTPUT_SD_FILE;
    new_l_append  = false;
  }
  else if(selection.startsWith("Log File (append)")) {
    new_l_kind    = LOG_OUTPUT_SD_FILE;
    new_l_append  = true;
  }
  if((new_l_kind != l_kind) || (new_l_append  != l_append)) {
    int sz;
    debug("Writing new preferences: log kind = %d, log append = %s\n", new_l_kind, new_l_append ? "true" : "false");
    prefs.begin(M5SYS_PREFS_NAME);
    sz = prefs.putInt("log_kind",    new_l_kind);
    if(4 != sz) { error("failed to write log_kind to prefs\n"); }
    sz = prefs.putBool("log_append", new_l_append);
    if(1 != sz) { error("failed to write log_append to prefs\n"); }
    prefs.end();
    set_from_prefs();
  }
}


void Logger::set_from_prefs() {
  verbose("Logger::set_from_prefs()\n");
  Preferences           prefs;
  prefs.begin(M5SYS_PREFS_NAME, true);	// read-only
  m5SysLogLevel_t       l_level   = (m5SysLogLevel_t)prefs.getInt(     "log_level",  LOG_INFO);
  m5sysLogOutputKind_t  l_kind    = (m5sysLogOutputKind_t)prefs.getInt("log_kind",   LOG_OUTPUT_SERIAL);
  bool                  l_append  = prefs.getBool(                     "log_append", false);
  prefs.end();

  Print* output = &Serial;;
  switch(l_kind) {
    case LOG_OUTPUT_SERIAL:
      output = &Serial;
      break;
    case LOG_OUTPUT_SERIAL2:
      output = &Serial2;
      break;
    case LOG_OUTPUT_SD_FILE:
      const char* mode = l_append ? "a" : "w";
      Logger::log_file = SD.open(LOG_FILE_NAME, mode);
      output           = &log_file;
      break;
  }
  set(l_level, *output);
}


void Logger::output_log(const char* tag, const char* format, va_list argp) {
  char buffer[_LOGGER_BUFFER_SIZE];
  int len = strlen(tag);
  strcpy(buffer, tag);
  vsnprintf(&buffer[len], _LOGGER_BUFFER_SIZE-len, format, argp);
  Logger::log_device->print(buffer);
  if(Logger::log_file.available()) Logger::log_file.flush();  // necessary for SD files
}
