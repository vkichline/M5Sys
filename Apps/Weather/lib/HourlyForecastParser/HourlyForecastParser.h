#pragma once

#include <NWS.h>
#include "JsonListener.h"

class HourlyForecastParser: public JsonListener {
  public:
    HourlyForecastParser(int num_forecasts, Forecast** forecasts);
    virtual void  whitespace(char c);
    virtual void  startDocument();
    virtual void  key(String key);
    virtual void  value(String value);
    virtual void  endArray();
    virtual void  endObject();
    virtual void  endDocument();
    virtual void  startArray();
    virtual void  startObject();
  protected:
    void          construct_data(int index, const char* name, const char* value);
    Forecast**    forecasts;
    int           num_periods;
    bool          in_properties   = false;
    bool          in_periods      = false;
    bool          in_array        = false;
    int           current_period  = 0;
    String        last_key;
};
