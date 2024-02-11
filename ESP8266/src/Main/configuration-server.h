#ifndef CONFIGURATION_SERVER_H
#define CONFIGURATION_SERVER_H

#include <ESP8266WebServer.h>

#include "./types.h"

struct Configuration {
  int8 timezoneHoursOffset;
  uint8 timezoneMinutesOffset;

  bool isInDaylightSaving;

  uint8 displayBrightness;
  bool decreaseBrightnessDuringNight;
};

class ConfigurationServer {
  ESP8266WebServer server;
  Configuration& config;

  bool mustUpdateTime;

  public:
  ConfigurationServer(Configuration& c, int port): config(c), server(port) { }

  void setup();
  bool handleClient();
};

#endif