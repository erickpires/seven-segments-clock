#ifndef M_NTP_H
#define M_NTP_H 1

#include <WiFiUdp.h>

#include "./types.h"

const int NTP_PACKET_SIZE = 48;  // NTP timestamp is in the first 48 bytes of the message

class NtpUpdater {
  WiFiUDP& udpClient;

  uint32 updateStartTime;
  bool isWaitingResponse;
  
  void sendNTPpacket(IPAddress&);
  uint32 readNTPpacket();

  public:

  NtpUpdater(WiFiUDP& client) : udpClient(client) {
    isWaitingResponse = false;
    updateIsRunning = false;
  }

  uint32 unixEpoch;
  bool updateIsRunning;

  void startNtpUpdate();
  bool tick();
};

#endif