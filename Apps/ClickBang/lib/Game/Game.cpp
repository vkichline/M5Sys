#include "Game.h"


Game::Game(int num_turns) {
  this->num_turns = num_turns;
  board           = new Board();
  turn_num        = 0;
  score           = 0;
}


// This can be overridden to implement more complex scoring systems.
//
int Game::calc_points_for_turn() {
  return 1;
}

// Play num_turns turns of the game. Return the score.
// Don't end the game on a hot streak; keep playing until it ends.
//
int Game::play() {
  board->clear();
  turn_num  = 0;
  score     = 0;
  init_display();
  while (turn_num < num_turns || is_hot()) {
    int  button = wait_for_input();
    bool result = board->hit_test(button);
    int  points = result ? calc_points_for_turn() : 0;
    turn_num++;
    score += points;
    update_display(points);
  }
  final_display();
  return score;
}


int Game::wait_for_input() {
  while (true) {
    M5.update();
    if(M5.BtnA.wasPressed()) return 0;
    if(M5.BtnB.wasPressed()) return 1;
    if(M5.BtnC.wasPressed()) return 2;
    delay(10);
  }
}


void Game::init_display() {
  M5.Lcd.printf("New Game.\n");
}

void Game::update_display(int points) {
  M5.Lcd.printf("%d ", points);
}

void Game::final_display() {
  M5.Lcd.printf("\nScore: %d\n", score);
}