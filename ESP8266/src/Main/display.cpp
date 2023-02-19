#include "Arduino.h"

#include "./types.h"
#include "./display.h"

#define NOP()     __asm__ __volatile__("nop")

void outputDigit(uint8 digit, DigitSelection selection) {
  // NOTE: Using direct port access to avoid writing the digit bit by bit.
  // Implementation of digitalWrite: https://github.com/esp8266/Arduino/blob/master/cores/esp8266/core_esp8266_wiring_digital.cpp
  uint8 setBits = digit & 0b00001111;
  uint8 clearBits = (~digit) & 0b00001111;

  GPOS = (setBits << 12);
  GPOC = (clearBits << 12);

  // NOTE: The selection lines are always held HIGH (all ones).
  // To select a different digit, all we need to do is clear the
  // bits that must be LOW, wait the propagation delay using NOPs and
  // change the bit to HIGH again.

  uint32 clearMask = DigitsClearMask[selection];

  GPOC = clearMask;

  // NOTE: The minimum setup time for the CD4511 is 150ns.
  // Considering a clock of 80MHz, each instruction takes:1000000000ns/80000000Hz = 12.5ns.
  // Therefore, we need at least 150/12.5 = 12 NOPs. During experimentation, 20 NOPs were 
  // necessary to make the output stable.
  NOP();
  NOP();
  NOP();
  NOP();
  NOP();
  NOP();
  NOP();
  NOP();
  NOP();
  NOP();
  NOP();
  NOP();
  NOP();
  NOP();
  NOP();
  NOP();
  NOP();
  NOP();
  NOP();
  NOP();

  GPOS = DIGITS_OUTPUT_MASK;
}