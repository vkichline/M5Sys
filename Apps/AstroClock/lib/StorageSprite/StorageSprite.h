#ifndef __storage_sprite_h__
#define __storage_sprite_h__

// A sprite that can be saved to/loaded from SPIFFS

#include <M5Stack.h>

class StorageSprite : public TFT_eSprite {
  public:
  StorageSprite(const char* fName);
  bool saveToBmp();
  const char* name() { return fName; }

  protected:
  char  fName[32];
};

#endif // __storage_sprite_h__
