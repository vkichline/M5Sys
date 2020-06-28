#pragma once

#include <M5Stack.h>
#include <Game.h>

class LcdGame : public Game {
  static const int ROW_HEIGHT       = 20;
  static const int CELL_GAP         = 5;
  static const int RR_RADIUS        = 4;
  static const int TOP_OFFSET       = 4;
  static const int BOTTOM_SETBACK   = 28;
  static const int TEXT_COLOR       = WHITE;
  static const int COLD_BACKGROUND  = BLACK;
  static const int HOT_BACKGROUND   = RED;
  static const int HIT_TONE         = 1720;
  static const int MISS_TONE        = 220;
  static const int TONE_DURATION    = 50; // mS
  static const int TONE_VOLUME      = 2;  // 0 - 10
  public:
    explicit      LcdGame(int num_turns);

  protected:
    void          init_display()              override;
    void          update_display(int points)  override;
    void          final_display()             override;
    int           calc_points_for_turn()      override;

  private:
    int           background_color();
    void          draw_info_bar();
    void          render_row(int index, int v_position);

    unsigned long turn_start_time;
    long          fastest_play;
    bool          muted = true;
};
