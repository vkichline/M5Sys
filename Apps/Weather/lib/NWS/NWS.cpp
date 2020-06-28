#include <HTTPClient.h>
#include <JsonStreamingParser.h>
#include <JsonListener.h>
#include <HourlyForecastParser.h>
#include <M5Sys.h>
#include "NWS.h"

extern  M5Sys   m5sys;

NWS::NWS() {
  VERBOSE("NWS::NWS()");
  for(int i = 0; i < NWS_NUM_FORECASTS;        i++) forecasts[i]        = nullptr;
  for(int i = 0; i < NWS_NUM_HOURLY_FORECASTS; i++) hourly_forecasts[i] = nullptr;
  set_position(0.0, 0.0);
}


NWS::NWS(float lat, float lon) {
  for(int i = 0; i < NWS_NUM_FORECASTS; i++) forecasts[i] = nullptr;
  VERBOSE("NWS::NWS(%.4f, %.4f)\n", lat, lon);
  set_position(lat, lon);
}


void NWS::set_position(float lat, float lon) {
  VERBOSE("NWS::set_position(%.4f, %.4f)\n", lat, lon);
  latitude            = lat;
  longitude           = lon;
  forecast_url        = "";
  hourly_forecast_url = "";
  description         = "";
}


bool NWS::begin() {
  VERBOSE("NWS::begin()\n");
  String      url = "https://api.weather.gov/points/" + String(latitude) + "," + String(longitude);
  HTTPClient  http;

  INFO("Using url: %s\n", url.c_str());
  http.begin(url);
  http.addHeader("User-Agent", "(esp32-m5stack-weather-app)");
  int httpCode = http.GET();
  INFO("NWS::begin httpCode = %d\n", httpCode);
  if(httpCode == HTTP_CODE_OK) {
    StaticJsonDocument<128> filter;
    DynamicJsonDocument     doc(JSON_OBJECT_SIZE(4) + 1024);

    filter["properties"]["forecast"]                                = true;
    filter["properties"]["forecastHourly"]                          = true;
    filter["properties"]["relativeLocation"]["properties"]["city"]  = true;
    filter["properties"]["relativeLocation"]["properties"]["state"] = true;

    Stream& response = http.getStream();
    DeserializationError error = deserializeJson(doc, response, DeserializationOption::Filter(filter));
    if(error) {
      ERROR("NWS::begin: DeserializationError = %s\n", error.c_str());
      VERBOSE("> return false\n");
      http.end();
      return false;
    }
    Serial.println(doc.as<String>());
    forecast_url             = doc["properties"]["forecast"].as<String>();
    hourly_forecast_url      = doc["properties"]["forecastHourly"].as<String>();
    description              = doc["properties"]["relativeLocation"]["properties"]["city"].as<String>() +
                               String(", ") +
                               doc["properties"]["relativeLocation"]["properties"]["state"].as<String>();
    INFO("forcast url        = %s\n", forecast_url.c_str());
    INFO("hourly forcast url = %s\n", hourly_forecast_url.c_str());
    INFO("description        = %s\n", description.c_str());
    http.end();
    return true;
  }
  http.end();
  return false;
}


bool NWS::load_forecasts() {
  VERBOSE("NWS::load_forecasts()\n");
  HTTPClient  http;

  if(0 == forecast_url.length()) {
    ERROR("Call begin() before load_forecasts()\n");
    return false;
  }
  INFO("Using url: %s\n", forecast_url.c_str());
  http.begin(forecast_url.c_str());
  http.useHTTP10(true);   // Use HTTP 1.0 to avoid chunked response
  http.addHeader("User-Agent", "(esp32-m5stack-weather-app)");
  int httpCode = http.GET();
  INFO("NWS::load_forecasts httpCode = %d\n", httpCode);
  if(httpCode == HTTP_CODE_OK) {
    StaticJsonDocument<128> filter;
    DynamicJsonDocument     doc(JSON_OBJECT_SIZE(13 * 14 + 14) + 500 * 14);

    filter["properties"]["periods"] = true;
    Stream& response = http.getStream();
    DEBUG("http.getStream() returned\n");
    DeserializationError error = deserializeJson(doc, response, DeserializationOption::Filter(filter));
    DEBUG("Deserialization complete\n");
    if(error) {
      http.end();
      ERROR("NWS::load_forecasts: DeserializationError = %s\n", error.c_str());
      VERBOSE("> return false\n");
      return false;
    }
    http.end();
    build_forecast_list(doc);
    return true;
  }
  http.end();
  return false;
}


bool NWS::load_hourly_forecasts() {
  VERBOSE("NWS::load_hourly_forecasts()\n");
  HTTPClient  http;

  if(0 == hourly_forecast_url.length()) {
    ERROR("Call begin() before load_hourly_forecasts()\n");
    return false;
  }
  INFO("Using url: %s\n", hourly_forecast_url.c_str());
  http.begin(hourly_forecast_url.c_str());
  http.useHTTP10(true);   // Use HTTP 1.0 to avoid chunked response
  http.addHeader("User-Agent", "(esp32-m5stack-weather-app)");
  int httpCode = http.GET();
  INFO("NWS::load_hourly_forecasts httpCode = %d\n", httpCode);
  if(httpCode == HTTP_CODE_OK) {
    Stream& response = http.getStream();
    DEBUG("http.getStream() returned\n");
    JsonStreamingParser   parser;
    HourlyForecastParser  listener(NWS_NUM_HOURLY_FORECASTS, hourly_forecasts);
    parser.setListener(&listener);
    while(response.available()) { parser.parse(response.read()); }
    http.end();
    return true;
  }
  http.end();
  return false;
}


void NWS::build_forecast_list(DynamicJsonDocument doc) {
  VERBOSE("NWS::build_forecast_list(doc)\n");
  for(int i = 0; i < NWS_NUM_FORECASTS; i++) {
    // If we are reusing the array, clear it as we go
    if(nullptr != forecasts[i]) {
      DEBUG("Deleting previous forecast record\n");
      delete forecasts[i];
    }
    DEBUG("Creating forecast #%d\n", i);
    forecasts[i] = new Forecast();
    forecasts[i]->name           = doc["properties"]["periods"][i]["name"].as<String>();
    forecasts[i]->start_time     = convert_to_time(doc["properties"]["periods"][i]["startTime"].as<const char*>());
    forecasts[i]->end_time       = convert_to_time(doc["properties"]["periods"][i]["endTime"].as<const char*>());
    forecasts[i]->is_daytime     = doc["properties"]["periods"][i]["isDaytime"].as<bool>();
    forecasts[i]->temperature    = doc["properties"]["periods"][i]["temperature"].as<float>();
    forecasts[i]->wind_speed     = doc["properties"]["periods"][i]["windSpeed"].as<String>();
    forecasts[i]->wind_direction = convert_to_wind_direction(doc["properties"]["periods"][i]["windDirection"].as<const char*>());
    forecasts[i]->short_forecast = doc["properties"]["periods"][i]["shortForecast"].as<String>();
    forecasts[i]->long_forecast  = doc["properties"]["periods"][i]["detailedForecast"].as<String>();
    forecasts[i]->icon_url       = doc["properties"]["periods"][i]["icon"].as<String>();
  }
}


// Given a string like "2020-06-22T18:00:00-07:00", convert it to a long datetime format
//
time_t NWS::convert_to_time(const char* zulu) {
  if(!zulu) { ERROR("NWS::convert_to_time(nullptr)\n"); return 0; }
  VERBOSE("NWS::convert_to_time(%s)\n", zulu);
  int     year, month, day, hour, minute, second, offset = 0;
  struct  tm  t = {0};
  sscanf(zulu, "%d-%d-%dT%d:%d:%d-%d:00", &year, &month, &day, &hour, &minute, &second, &offset);
  DEBUG("sscanf resulted in %d, %d, %d, %d, %d, %d, %d\n", year, month, day, hour, minute, second, offset);
  t.tm_year   = year - 1900;
  t.tm_mon    = month - 1;
  t.tm_mday   = day;
  t.tm_hour   = hour;
  t.tm_min    = minute;
  t.tm_sec    = second;
  t.tm_isdst  = (7 != offset);  // HACKHACK
  return mktime(&t);
}


// Given a string like "NNE", convert it to a WDir
WDir NWS::convert_to_wind_direction(const char* compass_dir) {
  if(!compass_dir) { ERROR("NWS::convert_to_wind_direction(nullptr)\n"); return X; }
  VERBOSE("NWS::convert_to_wind_direction(%s)\n", compass_dir);
  if     (0 == strcmp("N",   compass_dir)) return N;
  else if(0 == strcmp("NNE", compass_dir)) return NNE;
  else if(0 == strcmp("NE",  compass_dir)) return NE;
  else if(0 == strcmp("ENE", compass_dir)) return ENE;
  else if(0 == strcmp("E",   compass_dir)) return E;
  else if(0 == strcmp("ESE", compass_dir)) return ESE;
  else if(0 == strcmp("SE",  compass_dir)) return SE;
  else if(0 == strcmp("SSE", compass_dir)) return SSE;
  else if(0 == strcmp("S",   compass_dir)) return S;
  else if(0 == strcmp("SSW", compass_dir)) return SSW;
  else if(0 == strcmp("SW",  compass_dir)) return SW;
  else if(0 == strcmp("WSW", compass_dir)) return WSW;
  else if(0 == strcmp("W",   compass_dir)) return W;
  else if(0 == strcmp("WNW", compass_dir)) return WNW;
  else if(0 == strcmp("NW",  compass_dir)) return NW;
  else if(0 == strcmp("NNW", compass_dir)) return NNW;
  return X; // default is X
}


// Given the name field of a forecast, return the corresponding forecast record
//
Forecast* NWS::get_forecast_by_name(const char* name) {
  VERBOSE("NWS::get_forecast_by_name(%s)\n", name);
  for(int i = 0; i < NWS_NUM_FORECASTS; i++) {
    if(0 == strcmp(name, forecasts[i]->name.c_str())) return forecasts[i];
  }
  return nullptr;
}


// Given the name field of an hourly forecast, return the corresponding forecast record
//
Forecast* NWS::get_hourly_forecast_by_name(const char* name) {
  VERBOSE("NWS::get_hourly_forecast_by_name(%s)\n", name);
  for(int i = 0; i < NWS_NUM_HOURLY_FORECASTS; i++) {
    if(0 == strcmp(name, hourly_forecasts[i]->name.c_str())) return hourly_forecasts[i];
  }
  return nullptr;
}


// str must point to a buffer of at least 4 characters.
//
void NWS::wdir_to_str(WDir dir, char* str) {
  VERBOSE("NWS::wdir_to_str(%d, str)\n", dir);
  switch(dir) {
    case N:   strcpy(str,  "N");   break;
    case NNE: strcpy(str,  "NNE"); break;
    case NE:  strcpy(str,  "NE");  break;
    case ENE: strcpy(str,  "ENE"); break;
    case E:   strcpy(str,  "E");   break;
    case ESE: strcpy(str,  "ESE"); break;
    case SE:  strcpy(str,  "SE");  break;
    case SSE: strcpy(str,  "SSE"); break;
    case S:   strcpy(str,  "S");   break;
    case SSW: strcpy(str,  "SSW"); break;
    case SW:  strcpy(str,  "SW");  break;
    case WSW: strcpy(str,  "WSW"); break;
    case W:   strcpy(str,  "W");   break;
    case WNW: strcpy(str,  "WNW"); break;
    case NW:  strcpy(str,  "NW");  break;
    case NNW: strcpy(str,  "NNW"); break;
    case X:   strcpy(str,  "X!");  break;
  }
}
