#define M5STACK_MPU6886
#include <M5Sys.h>
#include "IChing.h"

#define HEXINFO_BUFFER_SIZE   2048

extern  M5Sys                 m5sys;


IChing::IChing() {
  VERBOSE("IChing::IChing()\n");
  initialized = false;
}

// The ISAM database index is expected to be on SD, named /data/hexagrams.idx
// As it is only 512 bytes, we'll keep the entire index loaded at all times
// Return true if usable, false if broken
// Set 'initialized' if the index is usable
//
bool IChing::begin() {
  VERBOSE("IChing::begin()\n");
  initialized = false;
  M5.IMU.Init();

  File idx = SD.open("/data/hexagrams.idx", "r");
  if(!idx) {
    ERROR("IChing::begin(): failed to open ISAM index file\n");
    return false;
  }
  if(512 != idx.size()) {
    ERROR("IChing::begin(): index file unexpected size. Expected 512, got %d\n", idx.size());
    idx.close();
    return false;
  }
  idx.read((uint8_t*)&index, 512);
  idx.close();
  INFO("ISAM index file loaded\n");
  initialized = true;
  return true;
}


// Make sure the index can be used, then look up the offset to the hexagram
// identified by hexagram number (from the text.)
// Delegate the creation of the info record from ISAM file on SD
// Note that ID is radix one, not zero
//
HexagramInfo* IChing::hexagram_from_number(uint8_t id) {
  VERBOSE("IChing::hexagram_from_number(%d)\n", id);
  if(!initialized) {
    ERROR("Call to IChing::hexagram_from_number() without call to IChing::begin()\n");
    return nullptr;
  }
  return load_hexagram_from_offset(index[id-1].id_offset);
}


// Make sure the index can be used, then look up the offset to the hexagram
// identified by hexagram value (calculated from the line values.)
// Delegate the creation of the info record from ISAM file on SD
//
HexagramInfo* IChing::hexagram_from_binary(uint8_t val) {
  VERBOSE("IChing::hexagram_from_binary(%d)\n", val);
  if(!initialized) {
    ERROR("Call to IChing::hexagram_from_binary() without call to IChing::begin()\n");
    return nullptr;
  }
  return load_hexagram_from_offset(index[val].val_offset);
}


// The ISAM database file is expected to be on SD, named /data/hexagrams.isam
// Open the ISAM, move to the location provided, and read a line of text.
// The text should split into 13 fields on tab separators:
//    0       value       Sum of binary values of lines (1 for solid, 0 for broken, bottom = lsb)
//    1       id          Hexagram number in the Book of Changes
//    2       name        English translation of hexagram name
//    3       title       Romanized Chinese name of hexagram
//    4       judgement   The Judgement, with encoded newlines
//    5       image       The Image, with embedded newlines
//    6 - 11  lines       Text for changing lines, from bottom up, with embedded newlines
//    12      all         For hexagrams 1 and 2, when all lines are changing; else empty
// Return a HexagramInfo instance, or nullptr on error.
//
HexagramInfo* IChing::load_hexagram_from_offset(int offset) {
  VERBOSE("IChing::load_hexagram_from_offset(%d)\n", offset);
  // First, make sure the isam file exists and can be opened.
  File isam = SD.open("/data/hexagrams.isam", "r");
  if(!isam) {
    ERROR("IChing::load_hexagram_from_offset(): Failed to open ISAM datafile\n");
    return nullptr;
  }
  if(!isam.seek(offset)) {
    ERROR("IChing::load_hexagram_from_offset(): Failed to seek location %d\n", offset);
    isam.close();
    return nullptr;
  }

  // Currently the longest record is 1342 bytes. We'll allow up to HEXINFO_BUFFER_SIZE (2K), to be safe
  DEBUG("reading from ISAM\n");
  char* buffer = (char*)malloc(HEXINFO_BUFFER_SIZE);
  int tabs = read_record(isam, buffer, HEXINFO_BUFFER_SIZE);
  isam.close();
  // Make sure we got a valid record with 12 tab characters
  if(12 != tabs) {
    ERROR("IChing::load_hexagram_from_offset: record at offset %d returned %d tabs, expected 12\n", offset, tabs);
    return nullptr;
  }

  HexagramInfo* result = create_info_from_buffer(buffer);
  free(buffer);
  return result;
}


// Read until we hit a <cr> or run out of buffer.
// Chop the <cr>, always terminate.
// Return the number of tabs encountered.
//
int IChing::read_record(File isam, char* buffer, int buff_size) {
  VERBOSE("IChing::read_record(isam, buffer, %d)\n", buff_size);
  int pos   = 0;
  int tabs  = 0;
  while(pos <= HEXINFO_BUFFER_SIZE) {
    buffer[pos] = isam.read();
    if('\n' == buffer[pos]) break;
    if('\t' == buffer[pos]) tabs++;
    pos++;
  }
  buffer[pos] = '\0';
  VERBOSE("Data: %s\n", buffer);
  return tabs;
}


// NOTE: The buffer will be changed by strtok
//
HexagramInfo* IChing::create_info_from_buffer(char* buffer) {
  VERBOSE("IChing::create_info_from_buffer(buffer)\n");
  if(!buffer) {
    ERROR("IChing::create_info_from_buffer() called with empty buffer!\n");
    return nullptr;
  }
  HexagramInfo* result = new HexagramInfo();

  char* cp = strtok(buffer, "\t");
  if(nullptr == cp) { delete result; return nullptr; }
  result->value = atoi(cp);
  VERBOSE("*  Value     = %d\n", result->value);

  cp = strtok(nullptr, "\t");
  if(nullptr == cp) { delete result; return nullptr; }
  result->id = atoi(cp);
  VERBOSE("*  ID        = %d\n", result->id);

  cp = strtok(nullptr, "\t");
  if(nullptr == cp) { delete result; return nullptr; }
  result->name = String(cp);
  VERBOSE("*  Name      = %s\n", result->name.c_str());

  cp = strtok(nullptr, "\t");
  if(nullptr == cp) { delete result; return nullptr; }
  result->title = String(cp);
  VERBOSE("*  Title     = %s\n", result->title.c_str());

  cp = strtok(nullptr, "\t");
  if(nullptr == cp) { delete result; return nullptr; }
  result->judgement = replace_crs(String(cp));
  VERBOSE("*  Judgement = %s\n", cp);

  cp = strtok(nullptr, "\t");
  if(nullptr == cp) { delete result; return nullptr; }
  result->image = replace_crs(String(cp));
  VERBOSE("*  Image     = %s\n", cp);

  for(int line = 0; line < 7; line++) {
    // Arduino bug? For some reason, this returns nullptr when last char is tab, rather than emtpy string
    cp = strtok(nullptr, "\t");
    if(nullptr == cp) {
      if(6 == line && 2 < result->id) {
        // If it's the 7th (imaginary) line and not hexagram 1 or 2, point to (expected) empty string
        cp = buffer;
        buffer[0] = '\n';
      }
      else {
        delete result;
        return nullptr;
      }
    }
    result->lines[line] = replace_crs(String(cp));
    VERBOSE("   * Line %d  = %s\n", line, cp);
  }
  return result;
}


// Replace the "\n" sequences in the text buffer with newline characters
//
String IChing::replace_crs(String str) {
  VERBOSE("IChing::replace_crs(%s)\n", str.c_str());
  str.replace("\\n", "\n");
  return str;
}