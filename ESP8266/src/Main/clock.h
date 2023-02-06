#ifndef M_CLOCK_H
#define M_CLOCK_H 1

#include "./types.h"

class Clock {
  uint32 prevMillis;

  void addSecond();
  void addMinute();
  void addHour();

  void addDay();
  void addMonth();
  void addYear();

  uint8 daysInCurrentMonth();

  public:
  uint32 milliseconds;

  uint16 year;
  uint8 month;
  uint8 day;

  uint8 hours;
  uint8 minutes;
  uint8 seconds;

  bool isSyncronized;

  void syncronizeFromUnixEpoch(uint32 unixEpoch);
  void tick();

  Clock() {
    isSyncronized = false;
  }
};

bool isLeapYear(uint16 year);

#endif