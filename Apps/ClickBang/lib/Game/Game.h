#pragma once

/*
  The Game object owns a board, scans for input, and keeps score.
*/

#include <Board.h>

class Game {
  public:
    explicit Game(int num_turns);
    int   play();
    bool  is_hot() { return board->is_hot(); }
  
  protected:
    virtual void  init_display();
    virtual void  update_display(int score);  // Score is zero for a miss, 1 - 5 for worst to best hits
    virtual void  final_display();

    virtual int   calc_points_for_turn();     // After each turn, calculates what to add to score
    virtual int   wait_for_input();           // Returns 0, 1 or 2

    Board*  board;
    int     num_turns;
    int     turn_num;
    int     score;
};
