#include <M5Sys.h>
#include <NWS.h>

// Using the NWS and the latitude/longitude, look up the current
// and coming weather.
// Strategy: use https://api.weather.gov/points/{lat}/{lon} to acquire
// gridpoint data, extract properties/forecast and properties/forecast/hourly


M5Sys       m5sys;
NWS         nws;
Location_t* position    = nullptr;
Location_t  fall_back   = { "SeaTac", 47.4447199, -122.31361, 130 };


void display_forecast(Forecast* f) {
  VERBOSE("display_forecast(forecast)\n");
  if(f) {
    char buffer[32];
    struct tm * timeinfo;
    ez.header.show(f->name + "'s Forecast");
    ez.buttons.show("Back ## Forecast");
    ez.canvas.clear();
    ez.canvas.font(&FreeSans9pt7b);
    ez.canvas.lmargin(10);
    ez.canvas.y(30);
    timeinfo = localtime(&f->start_time);
    strftime(buffer, 31, "%D: ", timeinfo);
    ez.canvas.print(buffer);
    strftime(buffer, 31, "%I:%M%p", timeinfo);
    ez.canvas.print(buffer);
    ez.canvas.print(" - ");
    timeinfo = localtime(&f->end_time);
    strftime(buffer, 31, "%I:%M%p", timeinfo);
    ez.canvas.println(buffer);
    ez.canvas.print("Temperature");
    ez.canvas.x(135);
    ez.canvas.println(int(f->temperature));
    ez.canvas.print("Wind speed");
    ez.canvas.x(135);
    ez.canvas.println(f->wind_speed);
    ez.canvas.print("Wind direction");
    nws.wdir_to_str(f->wind_direction, buffer);
    ez.canvas.x(135);
    ez.canvas.println(buffer);
    ez.canvas.println();
    ez.canvas.println(f->short_forecast);
  }
  String result = ez.buttons.wait();
  DEBUG("results = %s\n", result);
  if(0 == result.compareTo("Forecast")) {
    ez.textBox(f->name + " Forecast", f->long_forecast);
  }
}


void get_forecasts() {
  VERBOSE("get_forecasts()\n");
  ezMenu m("Forecast");
  m.txtSmall();
  m.buttons("up # Back # select ## down#");
  if(nws.load_forecasts()) {
    for(int i = 0; i < NWS_NUM_FORECASTS; i++) {
      Forecast*  forecast = nws.get_forecast(i);
      m.addItem(forecast->name);
    }
    while(true) {
      if(0 == m.runOnce()) return;
      String result = m.pickName();
      DEBUG("get_forecasts: result = %s\n", result.c_str());
      Forecast* forecast = nws.get_forecast_by_name(result.c_str());
      display_forecast(forecast);
    }
  }
}

void get_hourly_forecasts() {
  VERBOSE("get_hourly_forecasts()\n");
  ezMenu m("Hourly Forecast");
  m.txtSmall();
  m.buttons("up # Back # select ## down#");
  if(nws.load_hourly_forecasts()) {
    for(int i = 0; i < NWS_NUM_HOURLY_FORECASTS; i++) {
      Forecast*  forecast = nws.get_hourly_forecast(i);
      m.addItem(forecast->name);
    }
    while(true) {
      if(0 == m.runOnce()) return;
      String result = m.pickName();
      DEBUG("get_forecasts: result = %s\n", result.c_str());
      Forecast* forecast = nws.get_hourly_forecast_by_name(result.c_str());
      display_forecast(forecast);
    }
  }
}


void setup() {
  m5sys.begin("Weather", NETWORK_CONNECTION_AUTO);
  ez.canvas.clear();
  while(!WiFi.isConnected()) {
    ez.canvas.print(".");
    delay(500);
  }
  ez.canvas.println("\nLoading Weather Info");
  position = m5sys.position.get_current();
  if(!position) position = &fall_back;
  nws.set_position(position->latitude, position->longitude);
  nws.begin();
}


void loop() {
  VERBOSE("loop()\n");
  ezMenu  m("Weather for " + nws.get_description());
  m.txtSmall();
  m.buttons("up # back | Home # select ## down #");
  m.addItem("Get Forecast", get_forecasts);
  m.addItem("Get Hourly Forecast", get_hourly_forecasts);
  while(m.runOnce()) {}
  m5sys.goHome();
}
