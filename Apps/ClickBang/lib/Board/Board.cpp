#include "Board.h"


// Create a board with ROWS rows. Set hot_streak to false,
// set cold_hits and hot_hits to zero.
//
Board::Board() {
  clear();
}


// Delete all rows.
//
Board::~Board() {
  for(int i = 0; i < ROWS; i++) {
    delete rows[i];
  }
}


// Get ready for a new game
//
void Board::clear() {
  // Delete rows, if they've been created yet.
  for(int i = 0; i < ROWS; i++) {
    Row* row = rows[i];
    if(row) delete row;
  }
  // Create new rows
  for(int i = 0; i < ROWS; i++) {
    rows[i] = new Row();
  }
  hot_streak  = false;
  cold_hits   = 0;
  hot_hits    = 0;
  longest_run = 0;
}



// Given an index 0 thru COLUMNS-1, test for a hit on row 0.
// If it's a hit:
//  pop row 0 and add a new row at the end
//  clear cold_hits
//  if on hot_streak:
//    if it's an inexact hit:
//      increment hot_hits
//      if it's an exact hit:
//        increment cold_hits
//      if cold_hits >= HOT_THRESHOLD
//        clear hot_hits
//      if hot_hits >= HOT_THRESHOLD
//        end hot streak
//  if not on hot_streak:
//    increment cold_hits
//    if cold_hits >= HOT_THRESHOLD
//      start hot_streak
// If it's not a hit:
//  clear hit_count
//  clear hot_streak
//  return false
bool Board::hit_test(int index) {
  if(rows[0]->hit_test(index, hot_streak)) {
    // It's a hit
    if(hot_streak) {
      run_length++;
      if(run_length > longest_run) {
        longest_run = run_length;
      }
      
      // If it was an exact hit, bump cold hits. If at threshold, reset hot_hits and cold_hits
      if(rows[0]->hit_test(index)) {
        cold_hits++;
        if(cold_hits >= COLD_THRESHOLD) {
          hot_hits = cold_hits = 0;
        }
      }
      // If it wasn't an exact hit, increment hot_hits counter
      else {
        hot_hits++;
        if(cold_hits >= COLD_THRESHOLD) {
          hot_hits = 0;
        }
        if(hot_hits >= HOT_THRESHOLD) {
          hot_streak = false;
          cold_hits  = hot_hits = 0;
        }
      }
    }
    else {
      // not on a hot streak
      cold_hits++;
      if(cold_hits >= COLD_THRESHOLD) {
        hot_streak = true;
        run_length = 0;
        cold_hits  = hot_hits = 0;
      }
    }
    pop_row();
    return true;
  }
  // Miss
  cold_hits  = hot_hits = 0;
  hot_streak = false;
  return false;
}


// Delete rows[0], shift all rows up, add a new row at end.
//
void Board::pop_row() {
  delete rows[0];
  for(int row = 1; row < ROWS; row++) rows[row-1] = rows[row];
  rows[ROWS-1] = new Row();
}
