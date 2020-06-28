#ifndef __clockface_h__
#define __clockface_h__

#include "StorageSprite.h"

#define TIMEZONE_STRLEN   32


//  Information about the current day, provided in response to a DAY_URL request.
//  This data changes only once a day.
//
struct TimeData {
  int           utc                       = -1;
  int           local                     = -1;
  int           solar                     = -1;
  char          timezone[TIMEZONE_STRLEN] = { 0 };
  double        jdate                     = 0.0;
  int           gmst                      = -1;
  int           lmst                      = -1;
  int           doy                       = -1;
  unsigned long acquisition               = 0;
};


//  Information about the current time, provided in response to a TIME_URL request.
//  The result of this call is different virtually every time it's called.
//
struct DayData {
  double  bmatAngle  = 0.0;
  double  bmntAngle  = 0.0;
  double  bmctAngle  = 0.0;
  double  srAngle    = 0.0;
  double  ssAngle    = 0.0;
  double  mrAngle    = 0.0;
  double  msAngle    = 0.0;
  double  eectAngle  = 0.0;
  double  eentAngle  = 0.0;
  double  eeatAngle  = 0.0;
};

class AstroClock {
  public:
                AstroClock();
  void          drawTime();
  void          drawClockFace();
  void          draw();
  bool          fetchTime();
  bool          fetchDay();

  int           getLocalTime()       { return T.local; }
  int           getSiderealTime()    { return T.lmst;  }

  TimeData      T;
  DayData       D;

  protected:
  void          fillSegment(int x, int y, int start_angle, int sub_angle, int r, unsigned int color);
  void          lineToEdge(double angle, int offset, uint32_t color);
  void          drawClockHand(double angle, uint32_t color, bool adorn);
  void          drawWedges();
  int           timeStringToSeconds(const char* tstring);
  double        timeStringToAngle(const char* tstring);
  double        secondsToAngle(int seconds);

  private:
  StorageSprite img;
  int           updated;
};

#endif // __clockface_h__
