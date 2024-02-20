#ifndef DHT_H
#define DHT_H

#define DHT_INIT_TIME 1000
#define DHT_START_TRANSFER_INTERVAL 20
#define DHT_READ_INTERVAL 5000

// NOTE: As per the datasheet, a LOW pulse is 28us and a high pulse is 70us.
// We will use 50us, as it is almost the middle point.
#define DHT_HIGH_INTERVAL_THRESHOLD 50

enum SensorType {
  DHT11,
  DHT22
};

class Dht {
  SensorType sensorType;

  uint32 prevMillis;
  bool isReady;
  bool hasOngoingRead;

  uint8 sensorPin;

  uint8 waitForState(uint8 state);
  void readDataBits(uint8 buffer[]);
  public:

  uint8 temperatureHigh;
  uint8 temperatureLow;
  
  uint8 humidityHigh;
  uint8 humidityLow;
  
  void setup();
  void tick(uint32 millis);

  Dht(uint8 p, SensorType t): sensorPin(p), sensorType(t) { }
};

#endif