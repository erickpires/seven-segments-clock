#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

#include "./types.h"
#include "./ntp.h"
#include "./clock.h"

#ifndef STASSID
#include "./env.h"
#endif

#define DEBUG_MODE 1

const char* ssid = STASSID;
const char* pass = STAPSK;
const unsigned int udpListenPort = 2390;

// A UDP instance to let us send and receive packets over UDP
WiFiUDP udpClient = WiFiUDP();

Clock clockData = Clock();
NtpUpdater updater = NtpUpdater(udpClient);

uint8 lastSecond;

void setup() {
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

  clockData.tick();

#if DEBUG_MODE
  if(clockData.seconds != lastSecond) {
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
  }
#endif
}

void synchronizeClockWithUnixEpoch(Clock& clock, uint32 unixEpoch) {
  const int32 timezoneDisplacementInSeconds = -(3 * 60 * 60);
  uint32 localTime = unixEpoch + timezoneDisplacementInSeconds;

  clock.syncronizeFromUnixEpoch(localTime);
}