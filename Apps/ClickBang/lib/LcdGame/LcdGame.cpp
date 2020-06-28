#include "../../include/ClickBang.h"
#include "LcdGame.h"


LcdGame::LcdGame(int num_turns) : Game(num_turns) {
  M5.Speaker.setVolume(TONE_VOLUME);
  fastest_play = 9999;
}


void LcdGame::init_display(){
  M5.Lcd.fillScreen(background_color());
  M5.Lcd.setTextSize(1);
  update_display(false);
  turn_start_time = millis();
  muted = false;
}


int LcdGame::background_color() {
  return is_hot() ? HOT_BACKGROUND : COLD_BACKGROUND;
}


// Note that the board is drawn with the lowest numbered index at the bottom.
//
void LcdGame::update_display(int points) {
  static int  last_background = 0;
  int         row, pos;

  // Beep high for a hit, low for a miss
  if(!muted && g_prefs->sound) {
    int hit_tone = (HIT_TONE - 500) + (points * 500); // Modulate the tone for score
    M5.Speaker.setVolume(TONE_VOLUME + points);
    M5.Speaker.tone(points ? hit_tone : MISS_TONE, TONE_DURATION);
  }

  // Draw background if it has changed
  if(last_background != background_color()) {
    last_background = background_color();
    M5.Lcd.fillScreen(last_background);
  }
  // Draw the rows, from top down
  for(row = ROWS - 1, pos = 0; row >= 0 ; row--, pos++) {
    render_row(row, pos * (ROW_HEIGHT + CELL_GAP) + TOP_OFFSET);
  }
  draw_info_bar();
}


void LcdGame::final_display() {
  // Let the last beep complete, then shut off speaker so we don't get a protracted beep.
  delay(TONE_DURATION);
  M5.Speaker.end();
  muted = true;

  M5.Lcd.setTextColor(TEXT_COLOR, BLACK);
  M5.Lcd.clear();
  M5.Lcd.setTextSize(2);

  // Save high scores and print congrats or game over
  bool already = false;
  if(score > g_prefs->highest_score) {
    g_prefs->highest_score = score;
    M5.Lcd.drawCentreString("High Score!", 160, 10, 4);
    already = true;
  }
  if(turn_num > g_prefs->most_plays) {
    g_prefs->most_plays = turn_num;
    if(!already) {
      M5.Lcd.drawCentreString("Longest Play!", 160, 10, 4);
      already = true;
    }
  }
  if(board->longest_run_in_game() > g_prefs->longest_run) {
    g_prefs->longest_run = board->longest_run_in_game();
    if(!already) {
      M5.Lcd.drawCentreString("Longest Run!", 160, 10, 4);
      already = true;
    }
  }
  if(!already) {
      M5.Lcd.drawCentreString("Game Over", 160, 10, 4);
  }
  g_prefs->save();

  M5.Lcd.setTextSize(1);
  M5.Lcd.setCursor(150, 70);
  M5.Lcd.print("Now");
  M5.Lcd.drawLine(150, 100, 200, 100, WHITE);
  M5.Lcd.setCursor(240, 70);
  M5.Lcd.print("High");
  M5.Lcd.drawLine(240, 100, 293, 100, WHITE);

  M5.Lcd.setCursor(8, 110);
  M5.Lcd.print("Score");
  M5.Lcd.setCursor(150, 110);
  M5.Lcd.print(score);
  M5.Lcd.setCursor(240, 110);
  M5.Lcd.print(g_prefs->highest_score);

  M5.Lcd.setCursor(8, 140);
  M5.Lcd.print("Turns");
  M5.Lcd.setCursor(150, 140);
  M5.Lcd.print(turn_num);
  M5.Lcd.setCursor(240, 140);
  M5.Lcd.print(g_prefs->most_plays);

  M5.Lcd.setCursor(8, 170);
  M5.Lcd.print("Fastest");
  M5.Lcd.setCursor(150, 170);
  M5.Lcd.printf("%0.3f", (float)fastest_play / 1000.0);
  M5.Lcd.setCursor(240, 170);
  M5.Lcd.printf("%0.3f", (float)g_prefs->fastest_play / 1000.0);

  M5.Lcd.setCursor(8, 200);
  M5.Lcd.print("Hot Streak");
  M5.Lcd.setCursor(150, 200);
  M5.Lcd.print(board->longest_run_in_game());
  M5.Lcd.setCursor(240, 200);
  M5.Lcd.print(g_prefs->longest_run);

  //delay(1000);
  wait_for_input();
  M5.Lcd.clear();
}


// Info bar should contain:
//  turn_num/num_turns
//  score
//  COLD_THRESHOLD - cold_hits or HOT_THRESHOLD - hot_hits, depending on mode
void LcdGame::draw_info_bar() {
  char  buff[16];
  int   y     = M5.Lcd.height() - BOTTOM_SETBACK;
  int   to_go = is_hot() ? board->hot_to_go() : board->cold_to_go();

  M5.Lcd.setTextColor(TEXT_COLOR, background_color());
  M5.Lcd.setCursor(4, y, 4);
  M5.Lcd.printf("%d/%d", turn_num, num_turns);

  itoa(score, buff, 10);
  M5.Lcd.drawCentreString(buff, 160, M5.Lcd.height() - BOTTOM_SETBACK, 4);

  int x = (to_go >= 10) ? 284 : 300;
  M5.Lcd.setCursor(x, M5.Lcd.height() - BOTTOM_SETBACK, 4);
  M5.Lcd.printf("%d", to_go);
}


void LcdGame::render_row(int index, int v_position) {
  int column_width = M5.Lcd.width() / COLUMNS;
  for(int column = 0; column < COLUMNS; column++) {
    int color = board->rows[index]->cells[column]->color();
    M5.Lcd.fillRoundRect(column * column_width + CELL_GAP / 2, v_position + CELL_GAP / 2, column_width - CELL_GAP, ROW_HEIGHT, RR_RADIUS, color);
  }
}


// The faster the user gets a cell right, the more points are awarded.
// Minimum: 1 point
int LcdGame::calc_points_for_turn() {
  unsigned long now     = millis();
  unsigned long diff    = now - turn_start_time;
  int           result  = 1;
  turn_start_time       = now;

  if(diff < fastest_play) {
    fastest_play = diff;
  }
  if(diff < g_prefs->fastest_play) {
    g_prefs->fastest_play = diff;
  }
  if     (diff <= 250) result = 5;
  else if(diff <= 400) result = 4;
  else if(diff <= 600) result = 3;
  else if(diff <= 800) result = 2;
  return result;
}
