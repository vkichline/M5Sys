#pragma once
#include "../lib/Prefs/Prefs.h"

#define COLUMNS         3   // how many Cells are in a Row
#define ROWS            8   // how many Rows are in a Board
#define COLD_THRESHOLD  3   // how many hits trigger a hot streak
#define HOT_THRESHOLD   8   // how long a hot streak lasts

// Global variables
extern const  int     g_colors[];
extern const  int     g_numColors;
extern        Prefs*  g_prefs;