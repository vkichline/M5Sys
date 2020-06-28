#pragma once

#include <FS.h>

// This class is designed to remove all the hidden files my Mac adds to the SD image.
// This is difficult to manage from the Mac end.
//
class SDCleaner {
  public:
    void 	      remove_hidden_files();
	protected:
    const char* nameFromPath(const char* path);
    void 				walkfiles(File dir);
    void 				remove_dir(File dir);
    int         folderDeleteCount;
    int         failCount;
    int         deletedCount;
};
