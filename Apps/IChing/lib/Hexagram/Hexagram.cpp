#include <M5Sys.h>
#include <IChing.h>
#include "Hexagram.h"

extern  M5Sys   m5sys;
extern  IChing  iching;


Hexagram::Hexagram() {
  VERBOSE("Hexagram::Hexagram()\n");
  view_state  = ORIGINAL_VIEW;
  info        = nullptr;
  mt          = "";
  for(int line = 0; line < 6; line++) lines[line] = 0;
}


// Override with UI for generating hexagram
void Hexagram::generate() {
  VERBOSE("Hexagram::generate()\n");
  if(info) delete info;
  for(int line = 0; line < 6; line++) lines[line] = random(4) + 6;
  view_state   = ORIGINAL_VIEW;
  uint8_t val  = get_hexagram_value(view_state);
  info         = iching.hexagram_from_binary(val);
  DEBUG("Generated: [%d %d %d %d %d %d]  Value = %d\n", lines[0], lines[1], lines[2], lines[3], lines[4], lines[5], val);
}


// Load the hexagram of the given hexagram number
bool Hexagram::load_from_id(uint8_t id) {
  VERBOSE("Hexagram::load_from_id(%d)\n", id);
  if(info) delete info;
  info = iching.hexagram_from_number(id);
  if(nullptr != info) {
    create_lines_from_value();
    return true;
  }
  return false;
}


// Load the hexagram indicated by the values of its lines (in view_state mode)
bool Hexagram::load_from_val(uint8_t val) {
  VERBOSE("Hexagram::load_from_val(%d)\n", val);
  if(info) delete info;
  info = iching.hexagram_from_binary(val);
  if(nullptr != info) {
    create_lines_from_value();
    return true;
  }
  return false;
}


// true if any lines are 6 or 9
bool Hexagram::is_transformable() {
  VERBOSE("Hexagram::is_transformable()\n");
  for(int line = 0; line < 6; line++) {
    if(6 == lines[line] || 9 == lines[line]) return true;
  }
  return false;
}


// Change the view. Changes the hexagram if is_transformable is true
void Hexagram::set_view(bool view) {
  VERBOSE("Hexagram::set_view(%s)\n", view ? "Transformed" : "Original");
  if(view == view_state) {
    DEBUG("Hexagram::set_view() to existing state. No change.\n");
    return;
  }
  if(nullptr != info) {
    // Invert all sixes and nines, then re-load from value
    for(int line = 0; line < 6; line++) {
      if     (6 == lines[line]) lines[line] = 9;
      else if(9 == lines[line]) lines[line] = 6;
    }
    info = iching.hexagram_from_binary(get_hexagram_value(ORIGINAL_VIEW));
  }
}


// ORIGINAL_VIEW or TRANSFORMED_VIEW
bool Hexagram::get_view() {
  VERBOSE("Hexagram::get_view()\n");
  return view_state;
}


// true if hexagram is 1 or 2 and all lines are changing
bool Hexagram::seventh_line_applies() {
  VERBOSE("Hexagram::seventh_line_applies()\n");
  if(nullptr != info) {
    if(1 == info->id || 2 == info-> id) {
      for(int line = 0; line < 6; line++) {
        if(7 == line || 8 == line) return false;
      }
      return true;
    }
  }
  return false;
}


// this returns the hexagram number, or id, as used in the Book of Changes
uint8_t Hexagram::get_number() {
  VERBOSE("Hexagram::get_number()\n");
  if(nullptr != info) return info->id;
  else return 0;  // invalid hexagram number
}


String& Hexagram::get_name() {
  VERBOSE("Hexagram::get_name()\n");
  if(nullptr != info) return info->name;
  else return mt;
}


String& Hexagram::get_title() {
  VERBOSE("Hexagram::get_title()\n");
  if(nullptr != info) return info->title;
  else return mt;
}


String& Hexagram::get_judgement() {
  VERBOSE("Hexagram::get_judgement()\n");
  if(nullptr != info) return info->judgement;
  else return mt;
}


String& Hexagram::get_image() {
  VERBOSE("Hexagram::get_image()\n");
  if(nullptr != info) return info->image;
  else return mt;
}


// Note that lines are radix 1
String& Hexagram::get_changing_line(uint8_t line) {
  VERBOSE("Hexagram::get_changing_line(%d)\n", line);
  if(nullptr != info) {
    if(     1 <= line || 6 >= line)               return info->lines[line-1];
    else if(7 == line and seventh_line_applies()) return info->lines[6];
  }
  return mt;
}


// Note that lines are radix 1
// value of a specific line: 6, 7, 8 or 9 (0 if uninitialized)
uint8_t Hexagram::get_line_value(uint8_t line) {
  VERBOSE("Hexagram::get_line_value(%d)\n", line);
  if(1 > line || 6 < line) return 0;  // impossible value
  if(info) return lines[line-1];
  return 0;
}


// calculate the binary values of the sum of lines w/o transformation
uint8_t Hexagram::get_hexagram_value(bool state) {
  VERBOSE("Hexagram::get_hexagram_value(%s)\n", state ? "Transformed" : "Original");
  uint8_t value = 0;
  for(int line = 5; line >= 0; line--) {
    uint8_t lval = lines[line];
    if(ORIGINAL_VIEW == view_state) {
      if(7 == lval || 9 == lval) value++;
    }
    else if(TRANSFORMED_VIEW == view_state) {
      if(6 == lval || 7 == lval) value++;
    }
    value <<= 1;
  }
  value >>= 1;
  return value;
}


// After loading by id or value, create an unchanging hexagram
void Hexagram::create_lines_from_value() {
  if(nullptr == info) return;
  uint8_t value = info->value;
  for(int line = 0; line < 6; line++) {
    lines[line] = (value & 1) ? 7 : 8;
    value >>= 1;
  }
}
