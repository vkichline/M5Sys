#include <M5Sys.h>
#include "DiffMeter.h"

extern M5Sys  m5sys;


DiffMeter::DiffMeter(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t max) {
  VERBOSE("DiffMeter::DiffMeter(x=%d, y=%d, h=%d, w=%d, max=%d)\n", x, y, height, width, max);
  this->x       = x;
  this->y       = y;
  this->height  = height;
  this->width   = width;
  this->max     = max;
  this->value   = 0;
}


void DiffMeter::set_value(int16_t value) {
  VERBOSE("DiffMeter::set_value(%d)\n", value);
  if(abs(value) > max) {
    value = (value > 0) ? max : int16_t(max) * -1;
  }
  this->value = value;
}


// This is the first implementation, which is designed to be correct but not max efficient
//
void DiffMeter::draw() {
  VERBOSE("DiffMeter::draw()\n");
  bool      horz          = (width > height); // true if horizontal, false if vertical
  bool      plus          = (value > 0);      // true if center to right/down, false if center to left/up
  uint16_t  center        = horz ? width / 2 : height / 2;
  int16_t   scaled_value  = horz ? value * (width / 2) / max : value * (height / 2) / max;
  // Draw the outline in foreground color
  M5.Lcd.drawRect(x, y, width, height, ez.theme->foreground);
  DEBUG("DiffMeter::draw: frame x = %d, y = %d, w = %d, h = %d, c = %d\n", x, y, width, height, ez.theme->foreground);
  DEBUG("horz = %s, plus = %s, center = %d, scaled_value = %d\n", horz ? "True" : "False", plus ? "True" : "False", center, scaled_value);

  // fill the unused half with background color, in case the value has inverted polarity since last render
  if(horz) {
    if(plus) {
      M5.Lcd.fillRect(x+1, y+1, center-1, height-2, ez.theme->background);
      DEBUG("Horz + half blanking: x = %d, y = %d, w = %d, h = %d, c = %d\n", x+1, y+1, center-1, height-2, ez.theme->background);
    }
    else {
      M5.Lcd.fillRect(x+center, y+1, center-1, height-2, ez.theme->background);
      DEBUG("Horz - half blanking: x = %d, y = %d, w = %d, h = %d, c = %d\n", x+center, y+1, center-1, height-2, ez.theme->background);
    }
  }
  else {
    if(plus) {
      M5.Lcd.fillRect(x+1, y+center+1, width-2, center-2, ez.theme->background);
      DEBUG("Vert + half blanking: x = %d, y = %d, w = %d, h = %d, c = %d\n", x+1, y+center+1, width-2, center-2, ez.theme->background);
    }
    else {
      M5.Lcd.fillRect(x+1, y+1, width-2, center-2, ez.theme->background);
      DEBUG("Vert - half blanking: x = %d, y = %d, w = %d, h = %d, c = %d\n", x+1, y+1, width-2, center-1, ez.theme->background);
    }
  }

  // draw the solid indicator
  if(horz) {
    if(plus) {
      M5.Lcd.fillRect(x+center, y+1, scaled_value, height-2, ez.theme->foreground);
      DEBUG("Horz + fill: x = %d, y = %d, w = %d, h = %d, c = %d\n", x+center, y+1, scaled_value, height-2, ez.theme->foreground);
    }
    else {
      M5.Lcd.fillRect(x+center+scaled_value, y+1, abs(scaled_value), height-2, ez.theme->foreground);
      DEBUG("Horz - fill: x = %d, y = %d, w = %d, h = %d, c = %d\n", x+center+scaled_value, y+1, abs(scaled_value), height-2, ez.theme->foreground);
    }
  }
  else {
    if(plus) {
      M5.Lcd.fillRect(x+1, y+center-scaled_value, width-2, scaled_value, ez.theme->foreground);
      DEBUG("Vert + fill: x = %d, y = %d, w = %d, h = %d, c = %d\n", x+1, y+center-scaled_value, width-2, scaled_value, ez.theme->foreground);
    }
    else {
      M5.Lcd.fillRect(x+1, y+center, width-2, -scaled_value, ez.theme->foreground);
      DEBUG("Vert - fill: x = %d, y = %d, w = %d, h = %d, c = %d\n", x+1, y+center, width-2, -scaled_value, ez.theme->foreground);
    }
  }

  // erase the rest, in case the value has dropped.
  if(horz) {
    if(plus) {
      M5.Lcd.fillRect(x+center+scaled_value, y+1, (width/2)-scaled_value-1, height-2, ez.theme->background);
      DEBUG("Horz + tip erase: x = %d, y = %d, w = %d, h = %d, c = %d\n", x+center+scaled_value, y+1, (width/2)-scaled_value-1, height-2, ez.theme->background);
    }
    else {
      M5.Lcd.fillRect(x+1, y+1, center+scaled_value-1, height-2, ez.theme->background);
      DEBUG("Horz - tip erase: x = %d, y = %d, w = %d, h = %d, c = %d\n", x+1, y+1, (width/2)-scaled_value-1, height-2, ez.theme->background);
    }
  }
  else {
    if(plus) {
      M5.Lcd.fillRect(x+1, y+1, width-2, center-scaled_value-1, ez.theme->background);
      DEBUG("Vert + tip erase: x = %d, y = %d, w = %d, h = %d, c = %d\n", x+1, y+1, width-2, center-scaled_value-1, ez.theme->background);
    }
    else {
      M5.Lcd.fillRect(x+1, y+center-scaled_value+1, width-2, center+scaled_value-2, ez.theme->background);
      DEBUG("Vert - tip erase: x = %d, y = %d, w = %d, h = %d, c = %d\n", x+1, y+center-scaled_value+1, width-2, center+scaled_value-2, ez.theme->background);
    }
  }
}
