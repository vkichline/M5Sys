#include <M5Sys.h>
#include "SDCleaner.h"

extern  M5Sys   m5sys;


void SDCleaner::remove_hidden_files() {
  VERBOSE("SDCleaner::remove_hidden_files()\n");

  String response = ez.msgBox("Delete Hidden Files", "Delete all hidden files on the SD, such as resource forks and .spotlight directories.\n Are you sure?", "Cancel ## OK");
  DEBUG("response = %s\n", response);
  if(0 == response.compareTo("Cancel"))
    return;

  ez.header.show("Remove Hidden Files");
  ez.canvas.font(&FreeSans12pt7b);
  ez.canvas.clear();
  ez.canvas.y(35);
  ez.canvas.lmargin(20);
  ez.canvas.println("Deleting hidden files.");
  File root = SD.open("/");
  folderDeleteCount = failCount = deletedCount = 0;
  walkfiles(root);
  root.close();
  ez.canvas.println();
  ez.canvas.printf("%d hidden files deleted.\n", deletedCount);
  ez.canvas.println();
  ez.canvas.printf("%d directories deleted.\n", folderDeleteCount);
  ez.buttons.wait("Back");
  INFO("%d hidden files deleted.\n", deletedCount);
  INFO("%d directories deleted.\n",  folderDeleteCount);
  INFO("%d deletion failures.\n",    failCount);
}


const char* SDCleaner::nameFromPath(const char* path) {
  VERBOSE("SDCleaner::nameFromPath('%s')\n", path);
  const char* pos = strrchr(path, '/');
  if(!pos) return path;
  DEBUG("nameFromPath: '%s' -> '%s'\n", path, &pos[1]);
  return &pos[1];
}


void SDCleaner::walkfiles(File dir) {
  VERBOSE("SDCleaner::walkfiles(dir)\n");
  DEBUG("Walking directory %s\n", dir.name());
  while(File f =  dir.openNextFile()) {
    DEBUG("Processing %s\n", f.name());
    // Remove special directories
    if(f.isDirectory()) {
      String f_name(nameFromPath(f.name()));
      DEBUG("    %s is directory\n", f.name());
      //if('.' == f_name[0] && f_name != REGISTRY) {
      if('.' == f_name[0]) {
        remove_dir(f);
      }
      else {
        // just a normal directory
        walkfiles(f); // recurse
      }
    }
    else {
      // It's f file. Remove if hidden
      DEBUG("    %s is a file\n", f.name());
      if('.' == nameFromPath(f.name())[0]) {
        INFO("-F: %s\n", f.name());
        SD.remove(f.name());
        ez.canvas.print(".");
      }
    }
  }
}


void SDCleaner::remove_dir(File dir) {
  VERBOSE("SDCleaner::remove_dir(%s)\n", dir.name());
  DEBUG("Removing directory: %s\n", dir.name());
  File entry = dir.openNextFile();
  File nextEntry;
  while(true) {
    if(entry) {
      String localPath;
      if(entry.isDirectory()) {
        localPath = entry.name();
        nextEntry = dir.openNextFile();
        remove_dir(entry);
        if(SD.rmdir(localPath.c_str())) {
          INFO("-D: %s\n", localPath.c_str());
          ez.canvas.print(".");
          folderDeleteCount++;
        } 
        else {
          ERROR("Unable to delete folder: %s\n", localPath.c_str());
          failCount++;
        }
        entry = nextEntry;
      }
      else {
        localPath = entry.name();
        nextEntry = dir.openNextFile();
        if(SD.remove(localPath.c_str())) {
          INFO("-F: %s\n", localPath.c_str());
          ez.canvas.print(".");
          deletedCount++;
        } 
        else {
          ERROR("Failed to delete file: %s\n", localPath.c_str());
          failCount++;
        }
        entry = nextEntry;
      }
    }
    else {
      ez.canvas.print(".");
      // Strangely, most empty directories log an error like this if deleted:
      // [E][vfs_api.cpp:192] rmdir(): /.Spotlight-V100/Store-V2/5DCB5C8E-923F-4630-99C6-AF3CBFC88E63/journals.corespotlight does not exists or is a file
      // But not all! if the delete is skipped, you are left with .fsenvtsd and .Spotlight-V100, which are both root level.
      // Proposed logical solution: if there is a / in the name anywhere by name[0], do not delete the directory.
      String dir_name = dir.name();
      int index = strrchr(dir_name.c_str(), '/') - dir_name.c_str();
      DEBUG("Empty directory last / index = %d\n", index);
      if(0 == index) {
        INFO("-D: %s\n", dir_name.c_str());
        SD.rmdir(dir.name());
      }
      else {
        DEBUG("Skipping delete of non-root-level empty directory %s\n", dir_name.c_str());
      }
      folderDeleteCount++;
      break;
    }
  }
}
