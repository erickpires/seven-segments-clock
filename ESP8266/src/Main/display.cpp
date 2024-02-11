#include "core_esp8266_features.h"
#include <sigma_delta.h>
#include "Arduino.h"


#include "./types.h"
#include "./display.h"

#define TRANSITION_DURATION 256

#define SIGMA_DELTA_CHANNEL 0

enum TransitionState {
  DISPLAY_OLD_VALUE = 0,
  DISPLAY_NEW_VALUE = 1
};

void outputDigit(uint8 digit, uint selection);
TransitionState computeTransitionState(uint32 diff);

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
  sigmaDeltaSetup(SIGMA_DELTA_CHANNEL, 1220);
  sigmaDeltaAttachPin(D3, SIGMA_DELTA_CHANNEL);
}

void Display::setDigits(uint8 d[6]) {
  auto newValue = DisplayValue(d);

  if (newValue == this->currentDisplayValue) {
    return;
  }

  this->oldDisplayValue = this->currentDisplayValue;
  this->currentDisplayValue = newValue;

  this->hasNewDisplayValue = true;
}

void Display::setDotsState(DotsState state) {
  if (state == this->currentDotsState) {
    return;
  }
  
  this->oldDotsState = this->currentDotsState;
  this->currentDotsState = state;

  this->hasNewDotsState = true;
}

void Display::setBrightness(uint8 brightness) {
  sigmaDeltaWrite(SIGMA_DELTA_CHANNEL, brightness);
}

void Display::tick(uint32 millis) {
  if (this->hasNewDisplayValue) {
    // NOTE: These three assignments should be atomic to really
    // avoid race conditions. But, we know that 'isInDisplayTransition'
    // is always set and clear inside this function, which makes this 
    // function non-reentrant.
    // Since 'Display::tick' is always called from the main loop
    // once per iteration, we shouldn't have two calls racing against
    // themself. In other words, we should be safe having non-atomic
    // assignments.
    this->hasNewDisplayValue = false;
    this->isInDisplayTransition = true;
    this->displayTransitionInitialMillis = millis;
  }

  if (this->hasNewDotsState) {
    // NOTE: The same concern about the display assigments also apply here.
    this->hasNewDotsState = false;
    this->isInDotsTransition = true;
    this->dotsTransitionInitialMillis = millis;
  }

  // NOTE: Showing 'currentDisplayValue' should always be the default state.
  auto displayTransitionState = DISPLAY_NEW_VALUE;
  if (this->isInDisplayTransition) {
    uint32 millisDiff = millis - this->displayTransitionInitialMillis;

    displayTransitionState = computeTransitionState(millisDiff);

    if (millisDiff >= TRANSITION_DURATION) {
      this->isInDisplayTransition = false;
    }
  }

  for(uint i = 0; i < 6; i++) {
    if (displayTransitionState == DISPLAY_OLD_VALUE) {
      outputDigit(this->oldDisplayValue.digits[i], i);
    } else {
      outputDigit(this->currentDisplayValue.digits[i], i);
    }
  }

  auto dotsState = this->currentDotsState;
  if (this->isInDotsTransition) {
    uint32 millisDiff = millis - this->dotsTransitionInitialMillis;

    auto dotsTransitionState = computeTransitionState(millisDiff);
    if (dotsTransitionState == DISPLAY_OLD_VALUE) {
      dotsState = this->oldDotsState;
    }

    if (millisDiff >= TRANSITION_DURATION) {
      this->isInDotsTransition = false;
    }
  }

  outputDigit(dotsState, DOTS_SELECTION_INDEX);
}

// NOTE: We can conceptualize the transition animation as follows:
// - We take the transition duration of 256ms and divide it into 16 
//   chunks of 16ms each.
// - During the first chunk, we only show the old value.
// - During the second chunk, we show the new value for 1ms and the
//   old for 15ms.
// - All the way to the last chunk, where we show the new value for
//   15ms and the old value for 1ms.
// - After that, the transition is considered over and we only show
//   the new value.
// - This effectively gives us a transition with 16 discrete states
//   that are updated every 16ms (approximately 60HZ).
//
// This logic gives us a linear transition from the old value
// to the new. And because we used powers of 2, it can
// be implemented using only bitwise operations.
// A sigmoid curve would probably look better than a linear one,
// but this simple animation already gives a nice visual effect.
TransitionState computeTransitionState(uint32 diff) {
  if (diff >= TRANSITION_DURATION) {
    return DISPLAY_NEW_VALUE;
  }

  uint8 lowerNibble = diff & 0x0F;
  uint8 higherNibble = (diff >> 4) & 0x0F;

  if (lowerNibble >= higherNibble) {
    return DISPLAY_OLD_VALUE;
  }

  return DISPLAY_NEW_VALUE;
}

// TODO: Move the function to a platform especific file.
#define NOP()     __asm__ __volatile__("nop")
void outputDigit(uint8 digit, uint selection) {
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