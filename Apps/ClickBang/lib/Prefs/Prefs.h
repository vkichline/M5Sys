#pragma once


class Prefs {
  static const int    default_sound = true;
  static const int    default_turns = 100;

  public:
    Prefs();
    ~Prefs();

    // Operations on Preferences object:
    bool    begin();
    void    load();
    void    save();
    void    clear();

    // User settings
    bool    sound;          // True = on, false = off
    int     num_turns;      // Number of turns, initially 100

    // High scores, etc.
    int     highest_score;  // Highest scored game so far
    int     longest_run;    // Longest run in a game
    int     most_plays;     // Highest number of turns achieved in a game.
    int     fastest_play;   // Number of milliseconds delay in fastest response

  private:
    void    shaddow();      // Copy public values to backing store
    bool    is_dirty();     // Are public values different than shaddow values?

    // Shaddow copy of original values to see if write is necessary
    Prefs*  _shaddow;
    bool    started;
};
