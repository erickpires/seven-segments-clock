#include <ESP8266WiFi.h>
#include <time.h>

#include "./types.h"
#include "./clock.h"

void Clock::syncronizeFromUnixEpoch(uint32 unixEpoch) {
  time_t time = unixEpoch;

  struct tm c_time;
  gmtime_r(&time, &c_time);

  this->year = c_time.tm_year + 1900;
  this->month = c_time.tm_mon;
  this->day = c_time.tm_mday - 1;

  this->hours = c_time.tm_hour;
  this->minutes = c_time.tm_min;
  this->seconds = c_time.tm_sec;

  this->milliseconds = 0;
  this->prevMillis = millis();

  this->isSyncronized = true;
}

void Clock::tick() {
  // TODO: What happens when millis wrap around?
  auto currentMillis = millis();
  auto diff = currentMillis - this->prevMillis;

  this->milliseconds += diff;
  this->prevMillis = currentMillis;

  while(this->milliseconds >= 1000) {
    this->milliseconds -= 1000;
    this->addSecond();
  }
}

void Clock::addSecond() {
  this->seconds++;

  if(this->seconds >= 60) {
    this->seconds -= 60;
    this->addMinute();
  }
}

void Clock::addMinute() {
  this->minutes++;

  if(this->minutes >= 60) {
    this->minutes -= 60;
    this->addHour();
  }
}

void Clock::addHour() {
  this->hours++;

  if(this->hours >= 24) {
    this->hours -= 24;
    this->addDay();
  }
}

void Clock::addDay() {
  this->day++;

  if(this->day >= this->daysInCurrentMonth()) {
    this->day -= this->daysInCurrentMonth();
    this->addMonth();
  }

  // Synchronizing the clock once a day. 
  this->isSyncronized = false;
}

void Clock::addMonth() {
  this->month++;

  if(this->month >= 12) {
    this->month -= 12;
    this->addYear();
  }
}

void Clock::addYear() {
  this->year++;
}

uint8 Clock::daysInCurrentMonth() {
  // NOTE: Zero based.
  uint8 daysPerMonth[] = {
    30, // Jan
    0,  // Feb
    30, // Mar
    29, // Apr
    30, // May
    29, // Jun
    30, // Jul
    30, // Aug
    29, // Sep
    30, // Oct
    29, // Nov
    30  // Dec
  };

  if(this->month != 1) { // Not February
    return daysPerMonth[this->month];
  }

  return isLeapYear(this->year) ? 29 : 28;
}

bool isLeapYear(uint16 year) {
  // NOTE: It's very unlikely that this clock will be still working in 2100,
  // so I'm not implementing the full leap year rule. Also, even if it is work
  // in 2100, the clock should synchronize to the NTP server and correct the date.

  return (year & 0x03) == 0;
}