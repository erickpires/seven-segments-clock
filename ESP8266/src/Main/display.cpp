#include "core_esp8266_features.h"
#include "Arduino.h"

#include "./types.h"
#include "./display.h"

void Display::setup() {
  // Setting GPIO 12 through 15 as OUTPUT for the digit values.
  pinMode(12, OUTPUT);  // D6
  pinMode(13, OUTPUT);  // D7
  pinMode(14, OUTPUT);  // D5
  pinMode(15, OUTPUT);  // D8

  // Setting GPIO 2, 4 and 5 as OUTPUT for the selection digit lines.
  pinMode(2, OUTPUT); // D4
  pinMode(4, OUTPUT); // D2
  pinMode(5, OUTPUT); // D1

  // Setting all selection bits to HIGH, the default state.
  GPOS = DIGITS_OUTPUT_MASK;

  // Display brightness
  pinMode(D3, OUTPUT);
  digitalWrite(D3, HIGH);

  // sigmaDeltaSetup(0, 1220);
  // sigmaDeltaAttachPin(D3);
}

void Display::setDigits(uint8 d[6]) {
  for (uint i = 0; i < 6; i++) {
    this->digits[i] = d[i];
  }

  this->isDirty = true;
}

void Display::setDotsState(DotsState state, bool blinking) {
  this->dotsState = state;
  this->isBlinking = blinking;
}

void Display::tick(uint32 millis) {
  if (this->isDirty) {
    this->isDirty = false;
    for(uint i = 0; i < 6; i++) {
      this->outputDigit(this->digits[i], i);
    }
  }

  auto shouldBlankDots = millis & (1 << 9); // == (millis % 1024) < 512
  if (this->isBlinking && shouldBlankDots) {
    this->outputDigit(0x00, DOTS_SELECTION_INDEX);
  } else {
    this->outputDigit(this->dotsState, DOTS_SELECTION_INDEX);
  }  
}

#define NOP()     __asm__ __volatile__("nop")

void Display::outputDigit(uint8 digit, uint selection) {
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