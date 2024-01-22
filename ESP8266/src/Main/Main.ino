#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>


// #include <Adafruit_Sensor.h>
// #include <sigma_delta.h>

#include "./types.h"
#include "./ntp.h"
#include "./clock.h"
#include "./display.h"

#include "./dht.h"

#ifndef STASSID
#include "./env.h"
#endif

#define DEBUG_MODE 1

const char* ssid = STASSID;
const char* pass = STAPSK;
const unsigned int udpListenPort = 2390;

// A UDP instance to let us send and receive packets over UDP
WiFiUDP udpClient = WiFiUDP();

ESP8266WebServer server(8080);

Dht dht = Dht();
Clock clockData = Clock();
NtpUpdater updater = NtpUpdater(udpClient);

uint8 lastSecond;
uint8 displayDutyCicle = 127;
int32 timezoneOffsetInHours = -3;

void setup() {
  // Setting GPIO 12 through 15 as OUTPUT for the digit values.
  pinMode(12, OUTPUT);  // D6
  pinMode(13, OUTPUT);  // D7
  pinMode(14, OUTPUT);  // D5
  pinMode(15, OUTPUT);  // D8

  // Setting GPIO 2, 4 and 5 as OUTPUT for the selection digit lines.
  pinMode(2, OUTPUT); // D4
  pinMode(4, OUTPUT); // D2
  pinMode(5, OUTPUT); // D1

  // Dots and percent brightness
  pinMode(D3, OUTPUT);
  digitalWrite(D3, HIGH);

  outputDigit(0x00, DIGIT_DOTS);

  // Setting all selection bits to HIGH, the default state.
  GPOS = DIGITS_OUTPUT_MASK;

  dht.setup();

  // sigmaDeltaSetup(0, 1220);
  // sigmaDeltaAttachPin(D3);

  Serial.begin(115200);

#if DEBUG_MODE
  Serial.print("Connecting to ");
  Serial.println(ssid);
#endif

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

#if DEBUG_MODE
  Serial.print("\nConnected with IP address: ");
  Serial.println(WiFi.localIP());
#endif

  udpClient.begin(udpListenPort);

#if DEBUG_MODE
  Serial.println("\n\nStarted UDP client at port:");
  Serial.println(udpClient.localPort());
#endif

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

  dht.tick(millis());
  clockData.tick();

  if(clockData.seconds != lastSecond) {
#if DEBUG_MODE
    lastSecond = clockData.seconds;

    char buffer[5];
    Serial.print("The local time is ");

    sprintf(buffer, "%02d", clockData.day + 1);
    Serial.print(buffer);

    Serial.print('/');

    sprintf(buffer, "%02d", clockData.month + 1);
    Serial.print(buffer);

    Serial.print('/');

    sprintf(buffer, "%04ld", clockData.year);
    Serial.print(buffer);

    Serial.print(' ');

    sprintf(buffer, "%02d", clockData.hours);
    Serial.print(buffer);

    Serial.print(':');

    sprintf(buffer, "%02d", clockData.minutes);
    Serial.print(buffer);

    Serial.print(':');

    sprintf(buffer, "%02d", clockData.seconds);
    Serial.println(buffer);

    Serial.print(dht.temperatureHigh);
    Serial.print('.');
    Serial.print(dht.temperatureLow);
    Serial.print("º - ");
    Serial.print(dht.humidityHigh);
    Serial.print('.');
    Serial.print(dht.humidityLow);
    Serial.print('%');
#endif

    // sigmaDeltaWrite(0, displayDutyCicle);
  }


  outputDigit(clockData.seconds  % 10, DIGIT_SECONDS_0);
  outputDigit(clockData.seconds  / 10, DIGIT_SECONDS_1);

  outputDigit(clockData.minutes  % 10, DIGIT_MINUTES_0);
  outputDigit(clockData.minutes  / 10, DIGIT_MINUTES_1);

  outputDigit(clockData.hours  % 10, DIGIT_HOURS_0);
  outputDigit(clockData.hours  / 10, DIGIT_HOURS_1);

  // if (clockData.seconds % 2 == 0) {
  //   outputDigit(0x00, DIGIT_DOTS);
  // } else {
  //   outputDigit(0x0F, DIGIT_DOTS);
  // }

  server.handleClient();
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