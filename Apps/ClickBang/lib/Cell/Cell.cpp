#include "../../include/ClickBang.h"
#include "Cell.h"


// Create a cell with immutable color and 'special' attributes.
// Special case: if color is (the default) -1, a random color is selected.
//
Cell::Cell(int color, bool special) {
  _color   = color;
  _special = special;

  if(-1 == _color) {
    _color = g_colors[random(g_numColors)];
  }
}


// Return the color property
//
int Cell::color() {
  return _color;
}


// Return true if the 'special' attribute is set.
// Hit testing is looning for the special cell in a row.
//
bool Cell::hit_test() {
  return _special;
}
