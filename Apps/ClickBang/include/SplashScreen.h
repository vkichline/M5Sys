#pragma once

// Simple self-contained class to display a splash screen

#include <M5Stack.h>
#include "Settings.h"

class SplashScreen {
  public:
    int testButtons() {
      M5.update();
      if(M5.BtnA.wasPressed()) return 1;
      if(M5.BtnB.wasPressed()) return 2;
      if(M5.BtnC.wasPressed()) return 3;
      return 0;
    }

    void draw() {
      M5.Lcd.clear();
      M5.Lcd.setTextSize(2);

      M5.Lcd.drawCentreString("Click Bang", 160, 20, 4);
      M5.Lcd.drawCentreString("An Enjoyable Game", 160, 90, 2);
      M5.Lcd.drawCentreString("by Van Kichline", 160, 128, 2);
      
      M5.Lcd.drawFastHLine(0, 190, 320, M5.Lcd.color565(127,127,127));
      M5.Lcd.drawCentreString("Begin", 63, 200, 2);
      M5.Lcd.drawCentreString("Rules", 160, 200, 2);
      M5.Lcd.drawCentreString("Settings", 257, 200, 2);
    }

    void show() {
      draw();
      while(true) {
        int button = testButtons();
        switch(button) {
          case 1: return;
          case 2:
            display_help();
            while(!testButtons());
            draw();
            break;
          case 3:
            Settings* settings = new Settings();
            settings->show();
            g_prefs->save();
            draw();
            break;
        }
      }
    }

    void display_help() {
      M5.Lcd.clear();
      M5.Lcd.setTextSize(1);
      M5.Lcd.setTextWrap(true);
      M5.Lcd.setCursor(0, 0, 2);
      M5.Lcd.println("ClickBang is a simple game in which you make      points by guessing the target square in the       bottom row of colored boxes.");
      M5.Lcd.println("Each row has three boxes; one of one color, two of another color. The target square is always one of the two boxes of the same color. When you   hit the target by pressing the button below it, the row disappears and you win a point.");
      M5.Lcd.println("When you get several points in a row with no     misses, you are on a 'Hot Streak.' The background becomes red. Guessing either one of the two     squares of the same color earns a point.");
      M5.Lcd.println("Currently the game ends after 100 turns, but this is likely to change.");
      return;
    }
};
