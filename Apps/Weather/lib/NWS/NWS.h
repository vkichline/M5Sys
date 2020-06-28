#pragma once

#include <time.h>
#include <ArduinoJson.h>

#define   NWS_NUM_FORECASTS         14
#define   NWS_NUM_HOURLY_FORECASTS  12

enum WDir { N, NNE, NE, ENE, E, ESE, SE, SSE, S, SSW, SW, WSW, W, WNW, NW, NNW, X };

struct Forecast {
  String  name;
  time_t  start_time;
  time_t  end_time;
  bool    is_daytime;
  float   temperature;
  String  wind_speed;
  WDir    wind_direction;
  String  short_forecast;
  String  long_forecast;
  String  icon_url;
};


class NWS {
  public:
    NWS();
    NWS(float lat, float lon);
    bool      begin();
    void      set_position(float lat, float lon);
    bool      load_forecasts();
    bool      load_hourly_forecasts();
    Forecast* get_forecast(int index) { return forecasts[index]; }               // Do NOT delete the result!
    Forecast* get_hourly_forecast(int index) { return hourly_forecasts[index]; } // Do NOT delete the result!
    Forecast* get_forecast_by_name(const char* name);                            // Do NOT delete the result!
    Forecast* get_hourly_forecast_by_name(const char* name);                     // Do NOT delete the result!
    String&   get_description()       { return description; }
    time_t    convert_to_time(const char* zulu);
    WDir      convert_to_wind_direction(const char* compass_dir);
    void      wdir_to_str(WDir dir, char* str);
  protected:
    void      build_forecast_list(DynamicJsonDocument doc);
    float     latitude;
    float     longitude;
    String    forecast_url;
    String    hourly_forecast_url;
    String    description;
    Forecast* forecasts[NWS_NUM_FORECASTS];
    Forecast* hourly_forecasts[NWS_NUM_HOURLY_FORECASTS];
    friend class HourlyForecastParser;
};
