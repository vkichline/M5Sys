#include "Row.h"


// Create a row of COLUMNS cells. All but one is main_color,
// and one is of off_color. One of the main_color cells is special.
// The row is randomized after creation.
//
Row::Row() {
  assert(COLUMNS >= 2);
  main_color     = random_color();
  int off_color  = main_color;
  while(main_color == off_color) off_color = random_color();

  // Start with the special, main_color cell.
  cells[0] = new Cell(main_color, true);
  // Next, add the off_color cell.
  cells[1] = new Cell(off_color);
  // Make the rest of the cells, if any, main_color.
  for(int col = 2; col < COLUMNS; col++) {
    cells[col] = new Cell(main_color);
  }
  shuffle();
}


// Delete each cell object
//
Row::~Row() {
  for(int col = 0; col < COLUMNS; col++) {
    delete cells[col];
  }
}


// Randomize the order of the cells array.
//
void Row::shuffle()
{
  int last = 0;
  Cell* temp = cells[last];
  for (int i = 0; i < COLUMNS; i++) {
    int index = random(COLUMNS);
    cells[last] = cells[index];
    last = index;
  }
  cells[last] = temp;
}


// Given an index, determine if the indicated cell is 'special.'
// If in hot_streak mode, hitting any cell the same color as the
// special cell is a hit (self->main_color.)
// Hit returns true.
//
bool Row::hit_test(int column, bool hot_streak) {
  if(hot_streak) {
    if(cells[column]->color() == main_color) {
      return true;
    }
    return false;
  }
  else {
    return cells[column]->hit_test();
  }
}


int Row::random_color() {
  return g_colors[random(g_numColors)];
}