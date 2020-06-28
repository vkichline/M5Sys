#include <M5SysBase.h>
#include "StorageSprite.h"
#include <FS.h>
#include <SPIFFS.h>
#include "StorageSprite.h"
#include "WriteBmp.h"

extern  M5SysBase   m5sys;


StorageSprite::StorageSprite(const char* fName)
  : TFT_eSprite(&M5.Lcd) {
  VERBOSE("StorageSprite::StorageSprite(fName=%s)\n", fName);
  strlcpy(this->fName, fName, 31);
}


bool StorageSprite::saveToBmp() {
  VERBOSE("StorageSprite::saveToBmp()\n");
  return writeBmpFile(fName, _iwidth, _iheight, _img);
  
//  File file = SPIFFS.open(fName, "wb");
//  if(!file) {
//    ERROR("StorageSprite::save(): Failed to open file %s\n", fName);
//    return false;
//  }
//  if(16 == _bpp) {
//    file.write((byte*)_img, _iwidth * _iheight * 2);
//    saved = true;
//    return true;
//  }
//  else if(8 == _bpp) {
//    file.write((byte*)_img8, _iwidth * _iheight);
//    saved = true;
//    return true;
//  }
//  else if(1 == _bpp) {
//    file.write((byte*)_img8, (_iwidth >> 3) * _iheight + 1);
//    saved = true;
//    return true;
//  }
//  file.close();
//  return false;
}
