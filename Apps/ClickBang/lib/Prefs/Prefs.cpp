#include "Prefs.h"
#include <Preferences.h>

Preferences preferences;


// Don't load Preferences in constructor: M5.begin() must run first.
// Call Prefs::begin() before any other calls.
//
Prefs::Prefs() {
  started = false;
}


Prefs::~Prefs() {
  save();  // It's only written if settings have changed.
  preferences.end();
}


bool Prefs::begin() {
  started = preferences.begin("ClickBang");
  assert(started);
  load();
  return started;
}


void Prefs::clear() {
  assert(started);
  //preferences.remove("sound"); 
  //preferences.remove("num_turns");
  preferences.remove("highest_score");
  preferences.remove("longest_run");
  preferences.remove("most_plays");
  preferences.remove("fastest_play");
  load();
}


void Prefs::load() {
  assert(started);
  sound         = preferences.getBool("sound",        default_sound);
  num_turns     = preferences.getInt("num_turns",     default_turns);
  highest_score = preferences.getInt("highest_score", 0);
  longest_run   = preferences.getInt("longest_run",   0);
  most_plays    = preferences.getInt("most_plays",    0);
  fastest_play  = preferences.getInt("fastest_play",  9999);
  shaddow();
}


void Prefs::save() {
  assert(started);
  if(is_dirty()) {
    preferences.putBool("sound",        sound);
  //preferences.putInt("num_turns",     num_turns);
    preferences.putInt("highest_score", highest_score);
    preferences.putInt("longest_run",   longest_run);
    preferences.putInt("most_plays",    most_plays);
    preferences.putInt("fastest_play",  fastest_play);
  }
}


void Prefs::shaddow() {
  assert(started);
  _shaddow = new Prefs(*this);
}


bool Prefs::is_dirty() {
  assert(started);
  return (
    this->sound         != _shaddow->sound          ||
    this->num_turns     != _shaddow->num_turns      ||
    this->highest_score != _shaddow->highest_score  ||
    this->longest_run   != _shaddow->longest_run    ||
    this->most_plays    != _shaddow->most_plays     ||
    this->fastest_play  != _shaddow->fastest_play
  );
}
