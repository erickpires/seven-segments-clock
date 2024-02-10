#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

// #include <sigma_delta.h>

#include "./types.h"
#include "./ntp.h"
#include "./clock.h"
#include "./display.h"

#include "./dht.h"

#ifndef STASSID
#include "./env.h"
#endif

const char* ssid = STASSID;
const char* pass = STAPSK;
const unsigned int udpListenPort = 2390;

// A UDP instance to let us send and receive packets over UDP
WiFiUDP udpClient = WiFiUDP();

ESP8266WebServer server(8080);

Dht dht = Dht();
Clock clockData = Clock();
NtpUpdater updater = NtpUpdater(udpClient);
Display display = Display();

uint8 lastSecond;
uint8 lastHalfSecond;
uint8 displayDutyCicle = 127;
int32 timezoneOffsetInHours = -3;

enum DisplayMode {
  TIME,
  DATE,
  TEMPERATURE,
  HUMIDITY
};

void setup() {
  // NOTE: The circuit does not uses the TX/RX pins since they are
  // used to program the ESP8266. Therefore, there's no problem in
  // keeping the Serial configured in case we need it for debugging.
  Serial.begin(115200);

  // NOTE: This method sets the display pins direction and initial state.
  display.setup();
  dht.setup();

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  udpClient.begin(udpListenPort);

  setupConfigurationServer();
  server.begin();
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

  if(clockData.seconds != lastSecond) {
    lastSecond = clockData.seconds;

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
  }

  // NOTE: Instead of dividing the current milliseconds by 500 to
  //       get the halfSecond flag, we are dividing by 512 since it
  //       can be done with bitwise operations and is faster in some
  //       architectures. The 12 millisecond discrepancy should be
  //       visible anyway.
  uint8 currentHalfSecond = (clockData.milliseconds >> 9) & 1;
  if (currentHalfSecond != lastHalfSecond) {
    lastHalfSecond = currentHalfSecond;

    switch (displayMode) {
      case TIME:
        if (currentHalfSecond == 0) {
          display.setDotsState(BLANK);
        } else {
          display.setDotsState(DOUBLE_COLUMNS);
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
  }

  server.handleClient();
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

void synchronizeClockWithUnixEpoch(Clock& clock, uint32 unixEpoch) {
  const int32 timezoneOffsetInSeconds = (timezoneOffsetInHours * 60 * 60);
  uint32 localTime = unixEpoch + timezoneOffsetInSeconds;

  clock.syncronizeFromUnixEpoch(localTime);
}

void setupConfigurationServer() {
  server.on("/", []() {
    String content = "<!DOCTYPE HTML>\n<html><h1>7-segments clock configuration:</h1>";
    content += "<form method='get' action='settings'><label>Timezone hour offset: </label><input name='tz-offset' type='number' min='-12' max='12'><br/>";
    content += "<label>Display intensity: </label><input name='intensity' type='range' min='10' max='255'><br/>";
    content += "<input type='submit' value='Save settings'/></form>";
    content += "</html>";
    server.send(200, "text/html", content);  
  });
  server.on("/settings", []() {
    auto tzOffsetStr = server.arg("tz-offset");
    auto intensityStr = server.arg("intensity");

    auto tzOffset = tzOffsetStr.toInt();
    auto intensity = intensityStr.toInt();

    Serial.print("Offset: ");
    Serial.println(tzOffset);

    Serial.print("Intensity: ");
    Serial.println(intensity);

    if(intensity > 0 && intensity <= 255) {
      displayDutyCicle = (uint8) intensity;
    }

    if(tzOffset >= -12 && tzOffset <= 12) {
      timezoneOffsetInHours = tzOffset;
      clockData.isSyncronized = false;
    } 

    String content = "<!DOCTYPE HTML>\n<html><h1>Saved!</h1></html>";
    server.send(200, "text/html", content);
  });
}