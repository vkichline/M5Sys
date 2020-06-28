//The Game of Life, also known simply as Life, is a cellular automaton
//devised by the British mathematician John Horton Conway in 1970.
// https://en.wikipedia.org/wiki/Conway's_Game_of_Life

/*
   The MIT License (MIT)

   Copyright (c) 2016 RuntimeProjects.com

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in all
   copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.
*/

// Modified by Van Kichline
// May, in the year of the plague

#include <M5SysBase.h>

#define GRIDX         160
#define GRIDY         120
#define CELLXY        2
#define GEN_DELAY     0

long      genCount =  0;
uint8_t   grid[GRIDX][GRIDY];     // Current grid
uint8_t   newgrid[GRIDX][GRIDY];  // The new grid for the next generation
M5SysBase m5sys;                  // Application framework (lightly used)


//Draws the grid on the display
void drawGrid(void) {
  VERBOSE("drawGrid()\n");
  uint16_t color = TFT_WHITE;
  for (int16_t x = 1; x < GRIDX - 1; x++) {
    for (int16_t y = 1; y < GRIDY - 1; y++) {
      if ((grid[x][y]) != (newgrid[x][y])) {
        if (newgrid[x][y] == 1) color = 0xFFFF; //random(0xFFFF);
        else color = 0;
        M5.Lcd.fillRect(CELLXY * x, CELLXY * y, CELLXY, CELLXY, color);
      }
    }
  }
}


//Initialise Grid
void initGrid(void) {
  VERBOSE("initGrid()\n");
  for (int16_t x = 0; x < GRIDX; x++) {
    for (int16_t y = 0; y < GRIDY; y++) {
      newgrid[x][y] = 0;

      if (x == 0 || x == GRIDX - 1 || y == 0 || y == GRIDY - 1) {
        grid[x][y] = 0;
      }
      else {
        if (random(3) == 1)
          grid[x][y] = 1;
        else
          grid[x][y] = 0;
      }
    }
  }
}


// Check the Moore neighborhood
int getNumberOfNeighbors(int x, int y) {
  return grid[x - 1][y] + grid[x - 1][y - 1] + grid[x][y - 1] + grid[x + 1][y - 1] + grid[x + 1][y] + grid[x + 1][y + 1] + grid[x][y + 1] + grid[x - 1][y + 1];
}


//Compute the CA. Basically everything related to CA starts here
void computeCA() {
  VERBOSE("computeCA()\n");
  for (int16_t x = 1; x < GRIDX; x++) {
    for (int16_t y = 1; y < GRIDY; y++) {
      int neighbors = getNumberOfNeighbors(x, y);
      if (grid[x][y] == 1 && (neighbors == 2 || neighbors == 3 )) {
        newgrid[x][y] = 1;
      }
      else if (grid[x][y] == 1)  newgrid[x][y] = 0;
      if (grid[x][y] == 0 && (neighbors == 3)) {
        newgrid[x][y] = 1;
      }
      else if (grid[x][y] == 0) newgrid[x][y] = 0;
    }
  }
}


void generate() {
  VERBOSE("generate()\n");
  for (int16_t x = 1; x < GRIDX-1; x++) {
    for (int16_t y = 1; y < GRIDY-1; y++) {
      grid[x][y] = newgrid[x][y];
    }
  }
}


void display_info() {
  VERBOSE("display_info()\n");
  M5.Lcd.fillScreen(TFT_BLACK);

  int deadcells = 0;
  int livecells = 0;
  for (int16_t x = 1; x < GRIDX-1; x++) {
    for (int16_t y = 1; y < GRIDY-1; y++) {
      if(grid[x][y] == 0)
        deadcells++;
      else
        livecells++;
    }
  }
  M5.Lcd.setCursor(35, 20);
  M5.Lcd.print("Generation");
  M5.Lcd.setCursor(225, 20);
  M5.Lcd.println(genCount);
  M5.Lcd.setCursor(35, 45);
  M5.Lcd.print("Live Cells");
  M5.Lcd.setCursor(225, 45);
  M5.Lcd.println(livecells);
  M5.Lcd.setCursor(35, 70);
  M5.Lcd.print("Dead Cells");
  M5.Lcd.setCursor(225, 70);
  M5.Lcd.println(deadcells);
  M5.Lcd.setCursor(35, 95);
  M5.Lcd.print("Live/Dead");
  M5.Lcd.setCursor(225, 95);
  M5.Lcd.printf("%.2f%%", ((double(livecells) / double(deadcells))) * 100.0);

  M5.Lcd.setCursor(10, 200);
  M5.Lcd.println("Press any key to continue");
  m5sys.wait_for_any_button(true);
}


void splash() {
  VERBOSE("splash()\n");
  M5.Lcd.fillScreen(TFT_BLACK);
  M5.Lcd.setCursor(35, 25);
  M5.Lcd.println("Conway's Game of Life");

  M5.Lcd.setCursor(25, 90);
  M5.Lcd.println("Press A to Pause/Resume");
  M5.Lcd.setCursor(25, 115);
  M5.Lcd.println("Press B to Restart");
  M5.Lcd.setCursor(25, 140);
  M5.Lcd.println("Press C for Stats");
  M5.Lcd.setCursor(30, 200);
  M5.Lcd.println("Press any key to start");
  m5sys.wait_for_any_button();
}


void setup() {
  m5sys.begin("Life", NETWORK_CONNECTION_NONE);
  M5.Lcd.setTextColor(TFT_WHITE);
  M5.Lcd.setTextSize(2);
  splash();
}


void loop() {
  VERBOSE("loop()\n");
  bool  paused    = false;

  M5.Lcd.fillScreen(TFT_BLACK);
  initGrid();
  drawGrid();
  while(true) {
    if(paused) {
      delay(10);
    }
    else {
      computeCA();
      drawGrid();
      delay(GEN_DELAY);
      generate();
      genCount++;
    }

    M5.update();
    if(M5.BtnA.wasPressed()) {
      if(paused) {
        INFO("Game continuing.\n");
        paused = false;
      }
      else {
        INFO("Game paused.\n");
        paused = true;
        }
    }
    else if(M5.BtnB.wasPressed()) {
      INFO("Game restarted.\n");
      paused = false;
      genCount = 0;
      initGrid();
      drawGrid();
    }
    else if(M5.BtnC.wasPressed()) {
      INFO("Displaying stats.\n");
      display_info();
      M5.Lcd.fillScreen(TFT_BLACK);
      paused = false;
    }
  }
}
