#pragma once

#include <M5Stack.h>

/*
  Cell  (one cell per column)
    A Cell is one of COLUMNS objects in a horizontal Row.
    Its attributes are the color, and a boolean that determines if it is the
    target cell in the row.
    When hit tested, it returns true if the cell is 'special.'
    Row class ensures only one special cell per row.
*/

class Cell {
  public:
    Cell(int color = -1, bool special = false);
    int  color();
    bool hit_test();
  
  protected:
    int   _color;
    bool  _special;
};
