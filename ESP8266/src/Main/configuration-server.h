#ifndef CONFIGURATION_SERVER_H
#define CONFIGURATION_SERVER_H 1

#include <ESP8266WebServer.h>

#include "./configuration.h"

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