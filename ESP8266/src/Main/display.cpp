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

  delayMicroseconds(1);
  uint32 clearMask = DigitsClearMask[selection];

  GPOC = clearMask;

  delayMicroseconds(1);

  GPOS = DIGITS_OUTPUT_MASK;
}