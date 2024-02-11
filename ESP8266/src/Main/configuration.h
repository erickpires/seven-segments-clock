#ifndef CONFIGURATION_H
#define CONFIGURATION_H 1

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

#endif