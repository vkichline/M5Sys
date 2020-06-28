#ifndef __write_bmp_h__
#define __write_bmp_h__

#include <FS.h>
#include <SPIFFS.h>
#include <M5SysBase.h>

extern  M5SysBase   m5sys;


// Based on https://stackoverflow.com/questions/16724214/writing-images-with-an-arduino
// and https://web.archive.org/web/20080912171714/http://www.fortunecity.com/skyscraper/windows/364/bmpffrmt.html


bool writeBmpFile(const char* fName, int h, int w, const unsigned short* data) {
  File file = SPIFFS.open(fName, "w");
  if(!file) {
    ERROR("Failed to open file %s\n", fName);
    return false;
  }
  
  // set fileSize (used in bmp header)
  int rowSize  = 4 * ((3*w + 3)/4);     // how many bytes in the row (used to create padding)
  int fileSize = 54 + h*rowSize;        // headers (54 bytes) + pixel data

  // create padding (based on the number of pixels in a row
  unsigned char bmpPad[rowSize - 3*w];
  for (int i=0; i<sizeof(bmpPad); i++) {         // fill with 0s
    bmpPad[i] = 0;
  }

  // create file headers (also taken from StackOverflow example)
  unsigned char bmpFileHeader[14] = {            // file header (always starts with BM!)
    'B','M', 0,0,0,0, 0,0, 0,0, 54,0,0,0   };
  unsigned char bmpInfoHeader[40] = {            // info about the file (size, etc)
    40, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 24, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

  bmpFileHeader[ 2] = (unsigned char)(fileSize      );
  bmpFileHeader[ 3] = (unsigned char)(fileSize >>  8);
  bmpFileHeader[ 4] = (unsigned char)(fileSize >> 16);
  bmpFileHeader[ 5] = (unsigned char)(fileSize >> 24);

  bmpInfoHeader[ 4] = (unsigned char)(       w      );
  bmpInfoHeader[ 5] = (unsigned char)(       w >>  8);
  bmpInfoHeader[ 6] = (unsigned char)(       w >> 16);
  bmpInfoHeader[ 7] = (unsigned char)(       w >> 24);
  bmpInfoHeader[ 8] = (unsigned char)(       h      );
  bmpInfoHeader[ 9] = (unsigned char)(       h >>  8);
  bmpInfoHeader[10] = (unsigned char)(       h >> 16);
  bmpInfoHeader[11] = (unsigned char)(       h >> 24);

  file.write(bmpFileHeader, sizeof(bmpFileHeader));   // write file header
  file.write(bmpInfoHeader, sizeof(bmpInfoHeader));   // " info header

  for (int y = h - 1; y >= 0; y--) {                  // iterate image array
    unsigned char buffer[w * 3 + 4] = {0};
    for (int x = 0; x < w; x++) {
      short colorVal = data[y * w + x];
      colorVal = ((colorVal >> 8) & 0xff) + ((colorVal & 0xff) << 8);
      buffer[x * 3 + 2] = (colorVal & 0xf800) >> 8;   // R
      buffer[x * 3 + 1] = (colorVal & 0x07e0) >> 3;   // G
      buffer[x * 3 + 0] = (colorVal & 0x001f) << 3;   // B
    }
    file.write(buffer, w * 3 + (4-(w*3)%4)%4);        // and padding as needed
  }
  file.close();                                       // close file when done writing
  return true;
}

#endif // __write_bmp_h__
