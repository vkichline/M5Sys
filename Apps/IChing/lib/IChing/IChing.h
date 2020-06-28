#pragma once
#include <Arduino.h>
#include <SD.h>


struct IndexEntry {
  int   val_offset;     // offset to the hexagram record indexed by sum of lines
  int   id_offset;      // offset to the hexagram record indexed by hexagram number
};


struct HexagramInfo {
  uint8_t   id;         // Classic I Ching hexagram number (radix 1)
  uint8_t   value;      // binary order of hexagram based on line values (radix 0, different order)
  String    name;       // English translation of title
  String    title;      // Title in wade-giles romanization. Not the best system, but ASCII friendly
  String    judgement;  // Original judgement
  String    image;      // Confusion image
  String    lines[7];   // Comments on changing lines. #7 is all changing lines in hexagrams 1 and 2
};


class IChing {
  public:
    IChing();
    bool          begin();      // Load the isam idx
    HexagramInfo* hexagram_from_number(uint8_t id);
    HexagramInfo* hexagram_from_binary(uint8_t val);
  protected:
    IndexEntry    index[64];
    HexagramInfo* load_hexagram_from_offset(int offset);
    HexagramInfo* create_info_from_buffer(char* buffer);
    int           read_record(File isam, char* buffer, int buff_size);
    String        replace_crs(String str);
  private:
    bool          initialized;
};
