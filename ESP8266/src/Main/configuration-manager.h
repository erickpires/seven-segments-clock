#ifndef CONFIGURATION_MANAGER_H
#define CONFIGURATION_MANAGER_H

class ConfigurationManager {
  public:
  void setup();
  void writeConfiguration(Configuration& configuration);
  Configuration readConfiguration();
};

#endif