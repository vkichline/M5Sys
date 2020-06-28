#ifndef __planets_h__
#define __planets_h__

#include "StorageSprite.h"

#define UNINITED_MENU_COLOR   TFT_NAVY
#define ENABLED_MENU_COLOR    TFT_WHITE
#define DISABLED_MENU_COLOR   TFT_DARKGREY
#define SELECTED_MENU_COLOR   TFT_RED

#define WHATSUP_URL           "http://192.168.50.10:8080/whatsup"
#define NUM_PLANETS           7
#define MAX_PLANET            (NUM_PLANETS - 1)


// Information about a single planet, some provided by WHATSUP_URL, some by program.
//
struct Planet {
  Planet(const char* name, const char* abbr) {
    strlcpy(this->name, name, 31);
    this->abbreviation[0] = abbr[0];
    this->abbreviation[1] = abbr[1];
    this->abbreviation[2] = '\0';
  }
  char  name[32]          = { 0 };
  char  abbreviation[4]   = { 0 };
  bool  initialized       = false;;
  bool  visible           = false;;
  float alt;
  float azm;
  float distance;
  float illum;
};


class Planets {
  public:
                Planets();
  bool          anyPlanetsVisible();
  bool          anyPlanetSelected()   { return (-1 != currentPlanet); }
  bool          update();
  void          inc();
  void          dec();
  void          drawMenu();
  void          drawAltAzm();
  float         getAlt();
  float         getAzm();

  int           currentPlanet;
  Planet        planets[7]  = { Planet("Sun",    "Su"), Planet("Moon", "Mo"), Planet("Mercury", "Me"),
                                Planet("Venus",  "Ve"), Planet("Mars", "Ma"), Planet("Jupiter", "Ju"),
                                Planet("Saturn", "Sa") };

  protected:
  void          lineToEdge(double angle, int offset, uint32_t color, TFT_eSPI& view = M5.Lcd);
  void          drawAzm(float azm);
  void          drawAlt(float alt);

  private:
  const char*   emptyString = "";
  StorageSprite img;
};

#endif // __planets_h__
