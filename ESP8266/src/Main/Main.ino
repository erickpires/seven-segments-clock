#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

#include "./types.h"
#include "./ntp.h"
#include "./clock.h"
#include "./display.h"
#include "./configuration-server.h"
#include "./configuration-manager.h"

#include "./dht.h"

#ifndef STASSID
#include "./env.h"
#endif

const char* ssid = STASSID;
const char* pass = STAPSK;
const unsigned int udpListenPort = 2390;

Configuration configuration;
ConfigurationServer configServer = ConfigurationServer(configuration, 80);
ConfigurationManager configManager = ConfigurationManager();

// A UDP instance to let us send and receive packets over UDP
WiFiUDP udpClient = WiFiUDP();

Dht dht = Dht();
Clock clockData = Clock();
NtpUpdater updater = NtpUpdater(udpClient);
Display display = Display();

enum DisplayMode {
  TIME,
  DATE,
  TEMPERATURE,
  HUMIDITY
};

void setup() {
  // NOTE: This method sets the display pins direction and initial state.
  display.setup();
  dht.setup();

  configManager.setup();
  configuration = configManager.readConfiguration();

  // NOTE: The circuit does not uses the TX/RX pins since they are
  // used to program the ESP8266. Therefore, there's no problem in
  // keeping the Serial configured in case we need it for debugging.
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  udpClient.begin(udpListenPort);

  configServer.setup();
}

void loop() {
  if(!clockData.isSyncronized) {
    if(!updater.updateIsRunning) {
      updater.startNtpUpdate();
    } else {
      if(updater.tick()) {
        synchronizeClockWithUnixEpoch(clockData, updater.unixEpoch);        
      }
    }
  }

  auto currentMillis = millis();
  dht.tick(currentMillis);
  clockData.tick(currentMillis);
  display.tick(currentMillis);

  auto displayMode = modeToDisplay(clockData.seconds);
  switch (displayMode) {
    case TIME:
      displayTime();
      break;
    case DATE:
      displayDate();
      break;
    case TEMPERATURE:
      displayTemperature();
      break;
    case HUMIDITY:
      displayHumidity();
      break;
  }

  switch (displayMode) {
    case TIME:
      if (clockData.seconds % 2 == 0) {
        display.setDotsState(DOUBLE_COLUMNS);
      } else {
        display.setDotsState(BLANK);
      }
      break;
    case DATE:
      display.setDotsState(DOUBLE_PERIODS);
      break;
    case TEMPERATURE:
    case HUMIDITY:
      display.setDotsState(SINGLE_PERIOD);
      break;
  }

  if (isNightTime() && configuration.decreaseBrightnessDuringNight) {
    display.setBrightness(configuration.displayBrightness / 2);
  } else {
    display.setBrightness(configuration.displayBrightness);
  }

  auto mustUpdateConfigs = configServer.handleClient();

  if (mustUpdateConfigs) {
    clockData.isSyncronized = false;
    configManager.writeConfiguration(configuration);
  }
}

DisplayMode modeToDisplay(uint8 currentSeconds) {
  if (currentSeconds < 5) {
    return DATE;
  } else if (currentSeconds >= 30 && currentSeconds < 35) {
    return TEMPERATURE;
  } else if (currentSeconds >= 35 && currentSeconds < 40) {
    return HUMIDITY;
  }

  return TIME;
}

void displayTime() {
  uint8 displayDigits[8] = {
    clockData.seconds  % 10,
    clockData.seconds  / 10,

    clockData.minutes  % 10,
    clockData.minutes  / 10,

    clockData.hours  % 10,
    clockData.hours  / 10
  };

  display.setDigits(displayDigits);
}

void displayDate() {
  uint8 year = clockData.year  % 100;
  uint8 month = clockData.month + 1;
  uint8 day = clockData.day + 1;

  uint8 displayDigits[8] = {
    year  % 10,
    year  / 10,

    month  % 10,
    month  / 10,

    day  % 10,
    day  / 10
  };

  display.setDigits(displayDigits);
}

void displayTemperature() {
  uint8 displayDigits[8] = {
    dht.temperatureLow  % 10,
    dht.temperatureLow  / 10,

    dht.temperatureHigh  % 10,
    dht.temperatureHigh  / 10,

    DISPLAY_BLANK,
    DISPLAY_DEGREES
  };

  display.setDigits(displayDigits);
}

void displayHumidity() {
  uint8 displayDigits[8] = {
    dht.humidityLow  % 10,
    dht.humidityLow  / 10,

    dht.humidityHigh  % 10,
    dht.humidityHigh  / 10,

    DISPLAY_BLANK,
    DISPLAY_PERCENT
  };

  display.setDigits(displayDigits);
}

bool isNightTime() {
  return clockData.hours >= 21 || clockData.hours < 6;
}

void synchronizeClockWithUnixEpoch(Clock& clock, uint32 unixEpoch) {
  const int32 timezoneOffsetInSeconds = 
    (configuration.timezoneHoursOffset * 60 * 60) + 
    ((configuration.isInDaylightSaving ? 1 : 0) * 60 * 60) + 
    (configuration.timezoneMinutesOffset * 60);
  uint32 localTime = unixEpoch + timezoneOffsetInSeconds;

  clock.syncronizeFromUnixEpoch(localTime);
}
