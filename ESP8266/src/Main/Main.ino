#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

#include <Adafruit_Sensor.h>
#include <DHT.h>

#include "./types.h"
#include "./ntp.h"
#include "./clock.h"
#include "./display.h"

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

DHT dht(0, DHT11); // Using GPIO 0 = D3

// current temperature & humidity, updated in loop()
float t = 0.0;
float h = 0.0;

#define NOP()     __asm__ __volatile__("nop")

void setup() {
  // Setting GPIO 12 through 15 as OUTPUT for the digit values.
  pinMode(12, OUTPUT);
  pinMode(13, OUTPUT);
  pinMode(14, OUTPUT);
  pinMode(15, OUTPUT);

  // Setting GPIO 2, 4 and 5 as OUTPUT for the selection digit lines.
  pinMode(2, OUTPUT); // D4
  pinMode(4, OUTPUT); // D2
  pinMode(5, OUTPUT); // D1

  // Setting all selection bits to HIGH, the dafault state.
  GPOS = DIGITS_OUTPUT_MASK;

  dht.begin();

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

    t = dht.readTemperature();
    h = dht.readHumidity();

    Serial.print(t);
    Serial.print("ºC - ");
    Serial.print(h);
    Serial.println('%');
  }
#endif

  outputDigit(clockData.seconds  % 10, DIGIT_SECONDS_0);


}

void synchronizeClockWithUnixEpoch(Clock& clock, uint32 unixEpoch) {
  const int32 timezoneDisplacementInSeconds = -(3 * 60 * 60);
  uint32 localTime = unixEpoch + timezoneDisplacementInSeconds;

  clock.syncronizeFromUnixEpoch(localTime);
}