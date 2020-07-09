#pragma once

class BaseRenderer {
  public:
    virtual void draw_minimum() = 0;  // Call as often as possible, draws only what's needed
    virtual void draw_maximum() = 0;  // Call when settings change to redraw everything
    void draw_button_titles();        // Draw titles for the buttons at the bottom of the screen
};
