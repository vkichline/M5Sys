#define M5STACK_MPU6886
#include <Adafruit_NeoPixel.h>
#include <M5Sys.h>
#include "M5Hexagram.h"

extern  M5Sys                     m5sys;
extern  IChing                    iching;

#define LINE_LEFT                 90
#define LINE_WIDTH                140
#define LINE_PART_WIDTH           60
#define LINE_CENTER_GAP           20
#define LINE_HEIGHT               12
#define LINE_SPACING              12
#define HEXAGRAM_BOTTOM           185
#define HEXAGRAM_H_CENTER         160
#define CIRCLE_RADIUS             10
#define X_RADIUS                  14
#define HIGHLIGHT_COLOR           RED
#define COIN_VERTICAL             100
#define COIN_RADIUS               25
#define COIN_SPACING              60
#define M5STACK_FIRE_NEO_NUM_LEDS 10
#define M5STACK_FIRE_NEO_DATA_PIN 15
#define SHAKE_DELAY_MS            1000


enum sm_generate { wait_to_start, wait_to_end, wait_no_more };


M5Hexagram::M5Hexagram() : Hexagram() {
  VERBOSE("M5Hexagram::M5Hexagram(%d)\n", get_number());
  randomSeed(millis());
}


uint8_t M5Hexagram::value_from_coins(bool coins[]) {
  VERBOSE("M5Hexagram::value_from_coins(%d, %d, %d\n", coins[0], coins[1], coins[2]);
  return ((coins[0] ? 3 : 2) + (coins[1] ? 3 : 2) + (coins[2] ? 3 : 2));
}


// Given three coins, draw a representation on the screen.
//
void M5Hexagram::draw_coins(bool coins[]) {
  VERBOSE("M5Hexagram::draw_coins(%d, %d, %d\n", coins[0], coins[1], coins[2]);
  M5.Lcd.drawString(coins[0] ? "H " : "T ", HEXAGRAM_H_CENTER - COIN_SPACING - (coins[0] ? 9 : 7), COIN_VERTICAL - 8);
  M5.Lcd.drawString(coins[1] ? "H " : "T ", HEXAGRAM_H_CENTER                - (coins[1] ? 9 : 7), COIN_VERTICAL - 8);
  M5.Lcd.drawString(coins[2] ? "H " : "T ", HEXAGRAM_H_CENTER + COIN_SPACING - (coins[2] ? 9 : 7), COIN_VERTICAL - 8);
  M5.Lcd.drawCircle(HEXAGRAM_H_CENTER - COIN_SPACING, COIN_VERTICAL, COIN_RADIUS, ez.theme->foreground);
  M5.Lcd.drawCircle(HEXAGRAM_H_CENTER               , COIN_VERTICAL, COIN_RADIUS, ez.theme->foreground);
  M5.Lcd.drawCircle(HEXAGRAM_H_CENTER + COIN_SPACING, COIN_VERTICAL, COIN_RADIUS, ez.theme->foreground);
  M5.Lcd.drawString(String(value_from_coins(coins)), HEXAGRAM_H_CENTER - 7, COIN_VERTICAL + COIN_SPACING);
}


// The line number provided is radix 0
// Generate a random value 6, 7, 8 or 9 with the same
// distribution as the coins method for casting the I Ching.
//
uint8_t M5Hexagram::generate_line(uint8_t line) {
  VERBOSE("M5Hexagram::generate_line()\n");
  Adafruit_NeoPixel pixels  = Adafruit_NeoPixel(M5STACK_FIRE_NEO_NUM_LEDS, M5STACK_FIRE_NEO_DATA_PIN, NEO_GRB + NEO_KHZ800);
  sm_generate       state   = wait_to_start;
  bool              coins[3];
  unsigned long     start_millis;

  pixels.begin();
  ez.canvas.clear();
  ez.canvas.font(sans26);
  ez.canvas.pos(50, 36);
  ez.canvas.print("Shake ");
  ez.canvas.print(6 - line);
  ez.canvas.println(" more times");
  while(true) {
    float gyroX, gyroY, gyroZ = 0.0;
    ez.yield();
    switch(state) {
      case wait_to_start:
        M5.IMU.getGyroData(&gyroX, &gyroY, &gyroZ);
        if(30 < abs(gyroX)) {
          state         = wait_to_end;
          start_millis  = millis();
        }
        break;
      case wait_to_end:
        M5.IMU.getGyroData(&gyroX, &gyroY, &gyroZ);
        if(50 < abs(gyroX)) start_millis = millis();  // restart timer if still shaking
        coins[0] = 1 == random(2);
        coins[1] = 1 == random(2);
        coins[2] = 1 == random(2);
        draw_coins(coins);
        for(int pix = 0; pix < M5STACK_FIRE_NEO_NUM_LEDS; pix++) {
          pixels.setPixelColor(pix, pixels.Color(1<<random(0, 7), 1<<random(0, 7), 1<<random(0, 7)));          
        }
        pixels.show();
        if(1 > abs(gyroX) && millis() > start_millis + SHAKE_DELAY_MS) {
          state         = wait_no_more;
          start_millis  = millis();
        }
        break;
      case wait_no_more:
        for(int pix = 0; pix < M5STACK_FIRE_NEO_NUM_LEDS; pix++) pixels.setPixelColor(pix, BLACK);          
        pixels.show();
        delay(SHAKE_DELAY_MS);  // Display the value for a while
        return value_from_coins(coins);
    }
  }
  return 0;
}


// use accelerometer and neopixels to build the hexagram
void M5Hexagram::generate() {
  VERBOSE("M5Hexagram::generate()\n");
  ez.header.show("Create a Hexagram");
  // set all six values by shaking
  for(int line = 0; line < 6; line++) {
    lines[line] = generate_line(line);
  }
  if(info) delete info;
  view_state   = ORIGINAL_VIEW;
  uint8_t val  = get_hexagram_value(view_state);
  info         = iching.hexagram_from_binary(val);
  DEBUG("Generated: [%d %d %d %d %d %d]  Value = %d\n", lines[0], lines[1], lines[2], lines[3], lines[4], lines[5], val);
}


// if required, draw the appropriate highlight (O or X) centered at the vertical position provided
//
void M5Hexagram::draw_highlight(uint8_t value, uint16_t vert) {
  VERBOSE("M5Hexagram::draw_highlight(%d, %d\n", value, vert);
  if(6 == value) {
    // Draw an X
    M5.Lcd.drawLine(HEXAGRAM_H_CENTER - (X_RADIUS / 2),
                    (LINE_HEIGHT / 2) - (X_RADIUS / 2) + vert,
                    HEXAGRAM_H_CENTER + (X_RADIUS / 2),
                    (LINE_HEIGHT / 2) + (X_RADIUS / 2) + vert,
                    HIGHLIGHT_COLOR
                    );
    M5.Lcd.drawLine(HEXAGRAM_H_CENTER + (X_RADIUS / 2),
                    (LINE_HEIGHT / 2) - (X_RADIUS / 2) + vert,
                    HEXAGRAM_H_CENTER - (X_RADIUS / 2),
                    (LINE_HEIGHT / 2) + (X_RADIUS / 2) + vert,
                    HIGHLIGHT_COLOR
                    );
  }
  else if(9 == value) {
    // Draw an O
    M5.Lcd.drawCircle(HEXAGRAM_H_CENTER, vert + (LINE_HEIGHT / 2), CIRCLE_RADIUS, HIGHLIGHT_COLOR);
  }
}


void M5Hexagram::draw_hexagram(bool with_changes) {
  VERBOSE("M5Hexagram::draw_hexagram(%s)\n", with_changes ? "true" : "false");
  DEBUG("Rendering: [%d %d %d %d %d %d]\n", get_line_value(1), get_line_value(2), get_line_value(3), get_line_value(4), get_line_value(5), get_line_value(6));
  // draw from the bottom up
  uint16_t v = HEXAGRAM_BOTTOM - LINE_HEIGHT;
  for(int line = 1; line <= 6; line++) {
    uint8_t val = get_line_value(line);
    if(7 == val || 9 == val) {
      M5.Lcd.fillRect(LINE_LEFT, v, LINE_WIDTH, LINE_HEIGHT, ez.theme->header_bgcolor);
      if(with_changes) draw_highlight(val, v);
    }
    else if(6 == val || 8 == val) {
      M5.Lcd.fillRect(LINE_LEFT, v, LINE_PART_WIDTH, LINE_HEIGHT, ez.theme->header_bgcolor);
      M5.Lcd.fillRect(LINE_LEFT + LINE_PART_WIDTH + LINE_CENTER_GAP, v, LINE_PART_WIDTH, LINE_HEIGHT, ez.theme->header_bgcolor);
      if(with_changes) draw_highlight(val, v);
    }
    else {
      ERROR("M5Hexagram::draw_hexagram(): unexpected value for line #%d: %d\n", line, val);
      M5.Lcd.drawRect(LINE_LEFT, v, LINE_WIDTH, LINE_HEIGHT, HIGHLIGHT_COLOR);
    }
    v -= (LINE_HEIGHT + LINE_SPACING);
  }
}


void M5Hexagram::show(bool with_changes) {
  VERBOSE("M5Hexagram::show(%s)\n", with_changes ? "true" : "false");
  while(true) {
    String title(get_number());
    title.concat(": ");
    title.concat(get_name());
    ez.canvas.clear();
    ez.header.show(title);
    ez.buttons.show(is_transformable() ? "Back ## Jud # Img # Lns # Xfm" : "Back # Judgement # Image");
    draw_hexagram(with_changes);
    String result = ez.buttons.wait();
    DEBUG("result = %s\n", result.c_str());
    if(     0 == result.compareTo("Back"))  return;
    else if(     result.startsWith("Jud"))  ez.textBox(get_title(), get_judgement(), true);
    else if(     result.startsWith("Im"))   ez.textBox(get_title(), get_image(),     true);
    else if(0 == result.compareTo("Xfm"))   set_view(!get_view());
    else if(0 == result.compareTo("Lns")) {
      String text;
      for(int line = 0; line < 6; line++) {
        if(6 == lines[line] || 9 == lines[line]) {
          text += get_changing_line(line+1);  // be careful of the radix change!
          text += "\n\n";
        }
      }
      ez.textBox(get_name(), text, true);
    }
  }
}
