#include <EEPROM.h>

#include "./configuration.h"
#include "./configuration-manager.h"

  
void ConfigurationManager::setup() {
  EEPROM.begin(16);
}

void ConfigurationManager::writeConfiguration(Configuration& configuration) {
  EEPROM.write(0, configuration.timezoneHoursOffset);
  EEPROM.write(1, configuration.timezoneMinutesOffset);

  EEPROM.write(2, configuration.isInDaylightSaving);

  EEPROM.write(3, configuration.displayBrightness);
  EEPROM.write(4, configuration.decreaseBrightnessDuringNight);

  EEPROM.commit();
}

Configuration ConfigurationManager::readConfiguration() {
  Configuration result;

  result.timezoneHoursOffset = EEPROM.read(0);
  result.timezoneMinutesOffset = EEPROM.read(1);

  result.isInDaylightSaving = EEPROM.read(2);

  result.displayBrightness = EEPROM.read(3);
  result.decreaseBrightnessDuringNight = EEPROM.read(4);

  return result;
}
