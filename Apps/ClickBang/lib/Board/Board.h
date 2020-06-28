#pragma once

/*
  A ClickBang Board is a list of ROWS Rows.
  Special changes to True when the user gets Field.SpecialCount hits in a row.
*/

#include <Row.h>
#include "../../include/ClickBang.h"

class Board {
  public:
    Board();
    ~Board();
    void  clear();
    bool  hit_test(int index);
    bool  is_hot()              { return hot_streak; }
    int   hot_to_go()           { return HOT_THRESHOLD  - hot_hits;  }  // How many hits until hot streak is over?
    int   cold_to_go()          { return COLD_THRESHOLD - cold_hits; }  // How many hits before hot streak begins?
    int   longest_run_in_game() { return longest_run; }

    Row*  rows[ROWS] = { nullptr };
  
  protected:
    void  pop_row();
    
    bool  hot_streak;
    int   cold_hits;
    int   hot_hits;
    int   run_length;   // length of this run
    int   longest_run;  // length of the longest run in the game
};
