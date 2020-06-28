#include <M5Sys.h>
#include "HexDump.h"

extern  M5Sys   m5sys;


HexDump::HexDump(File file) : file(file) {
  VERBOSE("HexDump::HexDump(FS, %s)\n", file.name());
  offset  = 0;
  return;
}


char* HexDump::reverse(char* str) {
  for(int i = 0; i< strlen(str) / 2; i++) {
    char temp=str[i];
    str[i]=str[strlen(str)-i-1];
    str[strlen(str)-i-1]=temp;
  }
  return str;
}


void HexDump::dump_hex(int value, int digits, const char* followed_by) {
  VERBOSE("HexDump::dump_hex(%d, %d, '%s')\n", value, digits, followed_by);
  const char* convert   = "0123456789ABCDEF";
  char*       buffer    = (char*)malloc(digits + 1);
  if(buffer) {
    buffer[digits]  = '\0';
    for(int i = 0; i < digits; i++) {
      buffer[i] = convert[value & 0xF];
      value >>= 4;
    }
    reverse(buffer);
    ez.canvas.print(buffer);
    ez.canvas.print(followed_by);
    free(buffer);
  }
}


// Given a pointer to binary data, an offset into that data,
// and an address to print for the row, hexdump 8 bytes
// (unless limit is set, in that case, stop at that many bytes)
//
void HexDump::dump_row(const char* data, int offset, int address, int limit) {
  VERBOSE("HexDump::dump_row(data, offset=%d, address=%d, limit=%d)\n", offset, address, limit);
  int   addr_high = address >> 16;
  int   addr_low  = address & 0xFFFF;
  int   index;

  dump_hex(addr_high, 4, " ");
  dump_hex(addr_low,  4, " - ");
  for(index = 0; index < limit; index++) {
    dump_hex(data[index], 2, " ");
  }
  // If we stopped short of 8, fill the rest with blanks
  for(; index < 8; index++) {
    ez.canvas.print("   ");
  }
  ez.canvas.print("    ");
  for(index = 0; index < limit; index++) {
    ez.canvas.print(isPrintable(data[index]) ? data[index] : '.');
  }
  ez.canvas.println();
}


int HexDump::run() {
  VERBOSE("HexDump::run()\n");
  ez.header.show(file.name());
  ez.canvas.font(mono6x8);
  ez.canvas.lmargin(16);
 
  while(true) {
    char buffer[128];
    ez.canvas.clear();
    ez.canvas.y(54);
    if(!file.seek(offset)) {
      ERROR("HexDump::run: failed to seek to position %s\n", offset);
      return -2;
    }
    int bytes_read = file.read((uint8_t*)buffer, 128);
    DEBUG("bytes_read = %d\n", bytes_read);

    for(int addr = 0; addr < bytes_read; addr += 8) {
      dump_row(buffer + addr, addr, addr + offset, (bytes_read - addr < 8) ? (bytes_read - addr - 1) : 8);
    }
    String selection = ez.buttons.wait("up # Back # down");
    DEBUG("selection = %s\n", selection);
    if(0 == selection.compareTo("Back")) {
      return 0;
    }
    if(0 == selection.compareTo("up")) {
      if(offset >= 128)
        offset -= 128;
      else
        offset = 0;
      DEBUG("offset set to %d\n", offset);
    }
    if(0 == selection.compareTo("down")) {
      if(128 == bytes_read) offset += 128;
      DEBUG("offset set to %d\n", offset);
    }
  }
}
