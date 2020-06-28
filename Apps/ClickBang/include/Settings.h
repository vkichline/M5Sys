#pragma once

#include <M5Stack.h>
#include "ClickBang.h"


class Settings {
  public:
    int testButtons() {
      M5.update();
      if(M5.BtnA.wasPressed()) return 1;
      if(M5.BtnB.wasPressed()) return 2;
      if(M5.BtnC.wasPressed()) return 3;
      return 0;
    }

    void sound_prefs() {
      M5.Lcd.clear();
      M5.Lcd.setTextSize(2);

      M5.Lcd.drawCentreString("Sound", 160, 20, 4);
      M5.Lcd.drawFastHLine(0, 190, 320, M5.Lcd.color565(127,127,127));
      M5.Lcd.drawCentreString("On", 63, 200, 2);
      M5.Lcd.drawCentreString("Off", 257, 200, 2);

     while(true) {
        int button = testButtons();
        if(1 == button) {
          M5.Speaker.setVolume(3);
          M5.Speaker.tone(860, 100);
          g_prefs->sound = true;
          return;
        }
        else if (3 == button) {
          g_prefs->sound = false;
          return;
        }
      }
    }


    void clear_scores() {
      // Clear high scores?
      M5.Lcd.clear();
      M5.Lcd.drawCentreString("Clear Scores", 160, 20, 4);
      M5.Lcd.drawFastHLine(0, 190, 320, M5.Lcd.color565(127,127,127));
      M5.Lcd.drawCentreString("Clear", 63, 200, 2);
      M5.Lcd.drawCentreString("Skip", 257, 200, 2);
      
     while(true) {
        int button = testButtons();
        if(1 == button) {
          g_prefs->clear();
          return;
        }
        else if (3 == button) {
          return;
        }
      }
    }


    void show() {
      sound_prefs();
      clear_scores();
     }
};