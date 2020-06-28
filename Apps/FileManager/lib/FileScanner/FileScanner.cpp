#include <M5Sys.h>
#include "FileScanner.h"
#include <HexDump.h>

extern M5Sys  m5sys;

bool ascending_alpha_comp(const char* s1, const char* s2)                   { return 0 > strcmp(s1, s2); }
bool ascending_alpha_case_insensitive_comp(const char* s1, const char* s2)  { return 0 > strcasecmp(s1, s2); }


void FileScanner::menu(const char* title) {
  VERBOSE("FileScanner::scan_files(%s)\n", title);
  ezMenu  m(title);
  m.txtSmall();
  m.buttons("up # Back # select ##  down #");
  m.setSortFunction(ascending_alpha_case_insensitive_comp);
  
  File root = fs.open("/");
  create_menu(m, root);
  root.close();
  while(true) {
    if(0 == m.runOnce()) return;  // If Back was selected
    // A file was selected from the menu. Make sure it's just a file, and not a menu item with this\tthat syntax
    String selection = m.pickName();
    DEBUG("Selection = %s\n", selection.c_str());
    int    tabpos    = selection.indexOf('\t');
    if(0 < tabpos) {
      // Remove the tab and everyting after it
      int excess = selection.length() - tabpos;
      selection.remove(tabpos, excess);
    }
    // We need to replace the leading '/' which was stripped by the scanner for readability
    selection = "/" + selection;
    // If process_file returns true, this list of files has changed
    if(process_file(selection.c_str())) {
      while(m.deleteItem(1)) {}   // Delete all menu items
      File root = fs.open("/");
      create_menu(m, root);
      root.close();
    }
  }
}


bool FileScanner::isTextByExtension(const char* fname) {
  VERBOSE("FileScanner::isTextByExtension(%s)\n", fname);
  char* ext = strrchr(fname, '.');
  if(ext) {
    for(int i = 0; i < sizeof(text_extensions)/sizeof(char*); i++) {
      if(0 == strcasecmp(text_extensions[i], ext)) {
        return true;
      }
    }
  }
  return false;
}


void FileScanner::edit_file(const char* fname) {
  VERBOSE("FileScanner::file_info(%s)\n", fname);
  File target    = fs.open(fname, "r");
  String content = target.readString();
  target.close();
  ez.textBox(fname, content);
}


void FileScanner::file_info(const char* fname) {
  VERBOSE("FileScanner::file_info(FS, %s)\n", fname);
  char  buffer[32];
  File  file = fs.open(fname);
  if(!file) {
    ERROR("file_info: Failed to open file %s\n", fname);
    return;
  }
  int     fsize = file.size();
  time_t  ftime = file.getLastWrite();
  strftime(buffer, 32, "%Y-%m-%d %H:%M:%S", localtime(&ftime));
  String msg(fname);
  msg.concat("\n");
  msg.concat(fsize);
  msg.concat(" bytes\n");
  msg.concat(buffer);
  ez.msgBox("File Info", msg);
}


// Return true if the file was deleted
//
bool FileScanner::delete_file(const char* fname) {
  VERBOSE("FileScanner::delete_file(FS, %s)\n", fname);
  String msg = "Are you sure you want to delete \"";
  msg.concat(fname);
  msg.concat("\"?");
  String answer = ez.msgBox("Delete", msg, "Cancel ## OK");

  if(0 == answer.compareTo("OK")) {
    INFO("Deleting file %s\n", fname);
    // The target can be an empty directory, but never a full one.
    File file = fs.open(fname);
    if(file) {
      bool result;
      DEBUG("File open succeeded.\n");
      bool isdir = file.isDirectory();
      file.close();
      DEBUG("isdir = %s\n", isdir ? "true" : "false");
      if(isdir) {
        result = fs.rmdir(fname);
      }
      else {
        result = fs.remove(fname);
      }
      if(!result) {
        ERROR("FileScanner::delete_file: Failed to delete file %s\n", fname);
      }
      else {
        DEBUG("Delete successful.\n");
        return true;
      }
    }
    else {
      ERROR("FileScanner::delete_file: File open failed on '%s'. Unexpected.\n", fname);
    }
  }
  return false;
}


void FileScanner::hex_dump(const char* fname) {
  VERBOSE("FileScanner::hex_dump(FS, %s)\n", fname);
  File file = fs.open(fname);
  if(file) {
    DEBUG("Opened file %s\n", fname);
    HexDump hexdump(file);
    hexdump.run();
    file.close();
  }
  else {
    ERROR("FileScanner::hex_dump: failed to open file %s\n", fname);
  }
}


bool FileScanner::process_file(const char* fname) {
  VERBOSE("FileScanner::process_file(%s, FS)\n", fname);
  ezMenu m(fname);
  m.txtSmall();
  m.buttons("up # Back # select ##  down #");

  m.addItem("Info");
  m.addItem("Delete");
  if(isTextByExtension(fname)) m.addItem("Edit"); // Add Edit menu if this is a text file
  m.addItem("Hex Dump");

  while(true) {
    if(0 == m.runOnce())                  return false;
    String mname = m.pickName();
    DEBUG("Selected operation: %s\n", mname);
    if(0 == mname.compareTo("Info"))      file_info(fname);
    if(0 == mname.compareTo("Delete"))    if(delete_file(fname)) return true;
    if(0 == mname.compareTo("Edit"))      edit_file(fname);
    if(0 == mname.compareTo("Hex Dump"))  hex_dump(fname);
  }
}


// Given an empty menu and a directory, fill the menu with files and directory names for the SD/dirname.
// Files have their size appended to the right, directories have a / appended to the name.
// All file names are printed to the log.
//
void FileScanner::create_menu(ezMenu& m, File dir) {
  VERBOSE("FileScanner::scan(menu, %s)\n", dir.name());
  int fileCountInDir = 0;
  File file = dir.openNextFile();
  while(file) {
    String title(file.name());
    INFO("%s\n", title.c_str());

    fileCountInDir++;
    if(file.isDirectory()) {
      create_menu(m, file);
    }
    else {
      if(title.startsWith("/"))
        title.remove(0, 1);
      title += "\t";
      title += file.size();
      m.addItem(title);
    }
    file.close();
    file = dir.openNextFile();
  }
  // In case there were no files in the directory...
  if(0 == fileCountInDir) {
    String title = dir.name();
    if(title.startsWith("/"))
      title.remove(0, 1);
    INFO("%s is empty\n", title.c_str());
    title += "\t";
    title += "Empty";
    m.addItem(title);
  }
}
