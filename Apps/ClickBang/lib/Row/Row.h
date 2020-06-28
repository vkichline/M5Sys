#pragma once

#include <Cell.h>
#include "../../include/ClickBang.h"

class Row {
  public:
    Row();
    ~Row();
    bool  hit_test(int column, bool hot_streak = false);
  
    Cell* cells[COLUMNS] = { nullptr };

  protected:
    int   random_color();
    
    void  shuffle();
    int   main_color;
};
