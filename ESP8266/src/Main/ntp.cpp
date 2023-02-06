#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

#include "./types.h"
#include "./ntp.h"

#define DEBUG_MODE 1

#define sizeOfArray(arr) (sizeof(arr) / sizeof(arr[0]))
#define UPDATE_TIMEOUT 5000


void NtpUpdater::startNtpUpdate() {
  IPAddress timeServerIP;
  const char* ntpServerName = "a.st1.ntp.br";
  //  const char* ntpServerName = "time.nist.gov";

#if DEBUG_MODE
  Serial.println("Querying NTP server...");
#endif

  // TODO: Try multiple NTP servers if the main one is not responding
  WiFi.hostByName(ntpServerName, timeServerIP);
  this->sendNTPpacket(timeServerIP);

  this->updateStartTime = millis();
  this->isWaitingResponse = true;
  this->updateIsRunning = true;
}

bool NtpUpdater::tick() {
  if(!this->updateIsRunning) return false;
  
  int packetSize = this->udpClient.parsePacket();
  if (packetSize == 0) {
    uint32 timeSinceStart = millis() - this->updateStartTime;

    if(timeSinceStart > UPDATE_TIMEOUT) {
      this->startNtpUpdate();
    }    

    return false;
  } 

  this->unixEpoch = this->readNTPpacket();
  this->updateIsRunning = false;

#if DEBUG_MODE
  Serial.print("Received packet length: ");
  Serial.println(packetSize);

  Serial.print("Unix time = ");
  Serial.println(unixEpoch);
#endif

  return true;
}

// The NTP protocol works by sending an NTP packet to the Server and
// waiting for the server to respond with an NTP packet.
// For more details on the structure of the NTP packet see the following pages:
//
// https://labs.apnic.net/index.php/2014/03/10/protocol-basics-the-network-time-protocol/
// https://www.meinbergglobal.com/english/info/ntp-packet.htm
// http://what-when-how.com/computer-network-time-synchronization/ntp-packet-header-ntp-reference-implementation-computer-network-time-synchronization/

void NtpUpdater::sendNTPpacket(IPAddress& address) {
  uint8 packetBuffer[NTP_PACKET_SIZE] = {0};

  // Only setting the packet header as described here:
  // https://stackoverflow.com/a/23818643
  packetBuffer[0] = 0b11011011;  // LI=unsynchronized, Version=3, Mode=Client

  this->udpClient.beginPacket(address, 123);  // NTP requests are to port 123
  this->udpClient.write(packetBuffer, sizeOfArray(packetBuffer));
  this->udpClient.endPacket();
}

uint32 NtpUpdater::readNTPpacket() {
  uint8 packetBuffer[NTP_PACKET_SIZE] = {0};
  this->udpClient.read(packetBuffer, sizeOfArray(packetBuffer));

  uint32 secsSince1900 = (packetBuffer[40] << 24) | (packetBuffer[41] << 16) | (packetBuffer[42] << 8) | (packetBuffer[43]);

  const uint32 differenceToUnixEpoch = 2208988800UL; // (70 years + leap days) in seconds
  uint32 unixEpoch = secsSince1900 - differenceToUnixEpoch;

  return unixEpoch;
}