#pragma once

#include <IChing.h>

#define   ORIGINAL_VIEW       false
#define   TRANSFORMED_VIEW    true

// Represents a unique hexagram, potentially with changing lines

class Hexagram {
  public:
    Hexagram();
    virtual void  generate();                         // Override with UI for generating hexagram
    bool          load_from_id(uint8_t id);           // Load the hexagram of the given hexagram number
    bool          load_from_val(uint8_t val);         // Load the hexagram indicated by the values of its lines (in view_state mode)
    bool          is_transformable();                 // true if any lines are 6 or 9
    void          set_view(bool view=ORIGINAL_VIEW);  // Change the view. Changes the hexagram if is_transformable is true
    bool          get_view();                         // ORIGINAL_VIEW or TRANSFORMED_VIEW
    bool          seventh_line_applies();             // true if hexagram is 1 or 2 and all lines are changing
    uint8_t       get_number();                       // The rest depend on the view setting
    String&       get_name();
    String&       get_title();
    String&       get_judgement();
    String&       get_image();
    String&       get_changing_line(uint8_t line);
    uint8_t       get_line_value(uint8_t line);       // value of a specific line: 6, 7, 8 or 9 (0 if uninitialized)
    uint8_t       get_hexagram_value(bool state);     // calculate the binary values of the sum of lines w/ or w/o transformation
  protected:
    bool          view_state;                         // ORIGINAL_VIEW or TRANSFORMED_VIEW
    uint8_t       lines[6];                           // values may be 6, 7, 8 or 9
    HexagramInfo* info;                               // These are big, so one at a time
    void          create_lines_from_value();          // After loading by id or value, create an unchanging hexagram
    String        mt;
};
