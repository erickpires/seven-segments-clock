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

  bool operator!=(Configuration& other) {
    return 
      this->timezoneHoursOffset != other.timezoneHoursOffset ||
      this->timezoneMinutesOffset != other.timezoneMinutesOffset ||
      
      this->isInDaylightSaving != other.isInDaylightSaving ||

      this->displayBrightness != other.displayBrightness ||
      this->decreaseBrightnessDuringNight != other.decreaseBrightnessDuringNight;
  }
};

class ConfigurationServer {
  ESP8266WebServer server;
  Configuration& config;

  bool mustUpdateConfigs;

  void updateConfigFromServer();
  public:
  ConfigurationServer(Configuration& c, int port): config(c), server(port) { }

  void setup();
  bool handleClient();
};

#endif