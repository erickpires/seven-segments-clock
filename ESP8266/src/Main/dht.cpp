#include "core_esp8266_features.h"
#include "Arduino.h"

#include "./dht.h"

// Implemented according to: https://d229kd5ey79jzj.cloudfront.net/791/Datasheet_DHT11.pdf

void Dht::setup() {
  pinMode(DHT_PIN, INPUT);
}

void Dht::tick(uint32 millis) {
  uint32 millisDelta = millis - prevMillis;

  if(!isReady) {
    if(millisDelta > DHT_INIT_TIME) {
      isReady = true;
    } else {
      return; // Keep waiting for the 1 second init time.
    }
  } 

  if(hasOngoingRead) {
    if(millisDelta < DHT_START_TRANSFER_INTERVAL) {
      return;
    } 
  } else {
    if(millisDelta < DHT_READ_INTERVAL) {
      return;
    }

    // Pulling the data line LOW for >18ms to initialize the communication
    pinMode(DHT_PIN, OUTPUT);
    digitalWrite(DHT_PIN, LOW);

    prevMillis = millis;
    hasOngoingRead = true;

    return;    
  }

  prevMillis = millis;
  hasOngoingRead = false;

  // Letting the input pull-up pull the data line HIGH
  pinMode(DHT_PIN, INPUT);

  delayMicroseconds(50);

  noInterrupts(); // Disabling interrupts while reading the sensor bits

  uint8 data[40];
  readDataBits(data);

  interrupts(); // Re-enabling interrupts

  uint8 newTemperatureHigh = 0;
  uint8 newTemperatureLow = 0;
  uint8 newHumidityHigh = 0;
  uint8 newHumidityLow = 0;
  uint8 checksum = 0;

  for(int i = 0; i < 8; i++) {
    newHumidityHigh <<= 1;

    if(data[i] > DHT_HIGH_INTERVAL_THRESHOLD) {
      newHumidityHigh |= 1;
    }
  }

  for(int i = 8; i < 16; i++) {
    newHumidityLow <<= 1;

    if(data[i] > DHT_HIGH_INTERVAL_THRESHOLD) {
      newHumidityLow |= 1;
    }
  }

  for(int i = 16; i < 24; i++) {
    newTemperatureHigh <<= 1;

    if(data[i] > DHT_HIGH_INTERVAL_THRESHOLD) {
      newTemperatureHigh |= 1;
    }
  }

  for(int i = 24; i < 32; i++) {
    newTemperatureLow <<= 1;

    if(data[i] > DHT_HIGH_INTERVAL_THRESHOLD) {
      newTemperatureLow |= 1;
    }
  }

  for(int i = 32; i < 40; i++) {
    checksum <<= 1;

    if(data[i] > DHT_HIGH_INTERVAL_THRESHOLD) {
      checksum |= 1;
    }
  }

  uint8 sum = newTemperatureHigh + newTemperatureLow + newHumidityHigh + newHumidityLow;

  if(sum != checksum) {
    return;
  }

  temperatureHigh = newTemperatureHigh;
  temperatureLow = newTemperatureLow;
  humidityHigh = newHumidityHigh;
  humidityLow = newHumidityLow;
}

void Dht::readDataBits(uint8 buffer[]) {
  if(waitForState(HIGH) == -1) {
    return;
  }

  if(waitForState(LOW) == -1) {
    return;
  }

  
  for(int i = 0; i < 40; i++) {
    waitForState(HIGH);
    
    // Measuring the length of the HIGH pulse.
    buffer[i] = waitForState(LOW);
  }
}

uint8 Dht::waitForState(uint8 state) {
  uint32 startMicros = micros();
  while(digitalRead(DHT_PIN) != state) {
    if(micros() - startMicros > 255) {
      return 0xFF;
    }
  }

  return micros() - startMicros;
}