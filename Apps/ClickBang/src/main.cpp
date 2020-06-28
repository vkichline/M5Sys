#include <M5SysBase.h>
#include <LcdGame.h>
#include "ClickBang.h"
#include "SplashScreen.h"
#include "Prefs.h"

/*
  ClickBang is a minimal game for the M5Stack. There are multiple rows of three cells each.
  Within each row two cells are of the same color, and one is different. One of the two that
  share the same color is the target cell.
  The user presses one of three buttons, one under each cell. If it it the target cell,
  the row is deleted, rows drop down and a new row is generated. The 'streak count' is advanced
  by one (starting at zero.)
  If the user selects the wrong cell, a little time is wasted and a penalty is displayed. The
  'streak count' is reset to zero.
  If the 'streak count' reaches three, the user is 'on fire', and for a period TBD selecting
  either of the same-colored cells results in a hit.
  The game consists of timed frames. Scoring and frame termination are TBD.
*/

// No black and no red; these are used for background colors.
const int g_colors[]  = { /*TFT_BLACK,*/ TFT_NAVY, TFT_DARKGREEN, TFT_MAROON, TFT_PURPLE, TFT_OLIVE,
                          TFT_LIGHTGREY, TFT_DARKGREY, TFT_BLUE, TFT_GREENYELLOW, TFT_GREEN, TFT_YELLOW,
                          TFT_ORANGE, TFT_PINK, TFT_CYAN, TFT_DARKCYAN, /*TFT_RED,*/ TFT_MAGENTA, TFT_WHITE
                        };
const int g_numColors = sizeof(g_colors) / sizeof(int);
Prefs*    g_prefs     = new Prefs();
LcdGame*  game;
M5SysBase m5sys;


void setup() {
  m5sys.begin("ClickBang", NETWORK_CONNECTION_NONE);
  g_prefs->begin();
  game = new LcdGame(g_prefs->num_turns);
  SplashScreen* splash = new SplashScreen(); // a transient object with Lcd side effects
  splash->show();
  delete splash;
}


void loop() {
  VERBOSE("loop()\n");
  M5.Lcd.clear();
  M5.Lcd.setCursor(0, 0, 2);
  game->play();
}
