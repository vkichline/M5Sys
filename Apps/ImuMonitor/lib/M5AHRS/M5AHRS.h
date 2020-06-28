#pragma once

class M5AHRS {
  public:
    void    clear();
    void    show();
    void    zero();
  protected:
    void    draw_reticule();
    void    draw_bubble(int16_t x, int16_t y, uint16_t color);
    int16_t limit(int16_t value, uint16_t range);
    float   pitch             = 0.0F;
    float   roll              = 0.0F;
    float   yaw               = 0.0F;
    float   zero_offset_pitch = 0.0F;
    float   zero_offset_roll  = 0.0F;
};
