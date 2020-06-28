#include <M5Sys.h>
#include <JsonListener.h>
#include "HourlyForecastParser.h"

// This parser is designed to read an hourly forecast from the National Weather Service
// and extract the first 12 hourly periods. Unfortunately, 156 periods are returned,
// which is too much to handle.
// After a good deal of data, we will get a key named 'properties', then 'periods'.
// Immediately after this, the array of numbered periods begins, starting with '1'.

extern  M5Sys   m5sys;
extern  NWS     nws;


HourlyForecastParser::HourlyForecastParser(int num_forecasts, Forecast** forecasts) {
  VERBOSE("HourlyForecastParser::HourlyForecastParser(%d, forecasts\n", num_forecasts);
  this->num_periods = num_forecasts;
  this->forecasts   = forecasts;
  for(int i = 0; i < num_periods; i++) {
    if(forecasts[i]) delete(forecasts[i]);
    forecasts[i] = new Forecast();
  }
}


void HourlyForecastParser::whitespace(char c) {
  VERBOSE("HourlyForecastParser::whitespace(%c)\n", c);
}


void HourlyForecastParser::startDocument() {
  VERBOSE("HourlyForecastParser::startDocument()\n");
}


void HourlyForecastParser::key(String key) {
  VERBOSE("HourlyForecastParser::key(%s)\n", key.c_str());
  if(in_array) {
    // At this point, every name/value pair belongs to a forecast.
    last_key = key;
  }
  else {
    // we are still looking for the array, tick off nested objects:
    if(0 == key.compareTo("properties")) in_properties = true;
    else if(in_properties && (0 == key.compareTo("periods"))) in_periods = true;
  }
}

void HourlyForecastParser::value(String value) {
  VERBOSE("HourlyForecastParser::value(%s)\n", value.c_str());
  if(in_array) {
    // At this point, every name/value pair belongs to a forecast.
    if(0 == last_key.compareTo("number")) current_period = atoi(value.c_str());
    else {
      construct_data(current_period, last_key.c_str(), value.c_str());
    }
  }
}


void HourlyForecastParser::endArray() {
  VERBOSE("HourlyForecastParser::endArray()\n");
  if(in_array) in_array = false;
}


void HourlyForecastParser::endObject() {
  VERBOSE("HourlyForecastParser::endObject()\n");
}


void HourlyForecastParser::endDocument() {
  VERBOSE("HourlyForecastParser::endDocument()\n");
}


void HourlyForecastParser::startArray() {
  VERBOSE("HourlyForecastParser::startArray()\n");
  if(in_periods) in_array = true;
}


void HourlyForecastParser::startObject() {
  VERBOSE("HourlyForecastParser::startObject()\n");
}


// Be careful! Index is radix 1, not zero.
void HourlyForecastParser::construct_data(int index, const char* name, const char* value) {
  VERBOSE("HourlyForecastParser::construct_data(%d, %s, %s)\n", index, name, value);
  struct  tm * timeinfo;
  char    buffer[32];

  if(index > NWS_NUM_HOURLY_FORECASTS) return;
  if(0 == strcmp(name, "startTime")) {
    time_t t = nws.convert_to_time(value);
    INFO("time_t = %d\n", t);
    forecasts[index-1]->start_time = t;
    timeinfo = localtime(&t);
    strftime(buffer, 31, "Forecast for %I:%M%p", timeinfo);
    forecasts[index-1]->name = buffer;
  }
  else if(0 == strcmp(name, "endTime"))          forecasts[index-1]->end_time       = nws.convert_to_time(value);
  else if(0 == strcmp(name, "temperature"))      forecasts[index-1]->temperature    = atof(value);
  else if(0 == strcmp(name, "windSpeed"))        forecasts[index-1]->wind_speed     = value;
  else if(0 == strcmp(name, "windDirection"))    forecasts[index-1]->wind_direction = nws.convert_to_wind_direction(value);
  else if(0 == strcmp(name, "icon"))             forecasts[index-1]->icon_url       = value;
  else if(0 == strcmp(name, "shortForecast"))    forecasts[index-1]->short_forecast = value;
  else if(0 == strcmp(name, "detailedForecast")) forecasts[index-1]->long_forecast  = value;
}
