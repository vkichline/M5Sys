#pragma once

#include <Hexagram.h>


class M5Hexagram : public Hexagram {
  public:
    M5Hexagram();
    void    generate();
    void    show(bool with_changes=false);
  protected:
    uint8_t generate_line(uint8_t line);
    void    draw_hexagram(bool with_changes);
    void    draw_highlight(uint8_t value, uint16_t vert);
    void    draw_coins(bool coins[]);
    uint8_t value_from_coins(bool coins[]);
};
