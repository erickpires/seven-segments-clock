#ifndef DISPLAY_H
#define DISPLAY_H 1

enum DigitSelection {
  DIGIT_SECONDS_0 = 0,
  DIGIT_SECONDS_1 = 1,

  DIGIT_MINUTES_0 = 2,
  DIGIT_MINUTES_1 = 3,

  DIGIT_HOURS_0   = 4,
  DIGIT_HOURS_1   = 5,

  DIGIT_DOTS      = 6,

  NO_DIGITS       = 7
};

#define DIGITS_OUTPUT_MASK 0b00110100

// NOTE: Represents how to output a given selection by clearing some bits.
// Assumes all the relevant bits are already HIGH.
const uint32 DigitsClearMask[] = {
  (~0b00000000) & DIGITS_OUTPUT_MASK,
  (~0b00000100) & DIGITS_OUTPUT_MASK,
  (~0b00010000) & DIGITS_OUTPUT_MASK,
  (~0b00010100) & DIGITS_OUTPUT_MASK,
  (~0b00100000) & DIGITS_OUTPUT_MASK,
  (~0b00100100) & DIGITS_OUTPUT_MASK,
  (~0b00101000) & DIGITS_OUTPUT_MASK,
  (~0b00101100) & DIGITS_OUTPUT_MASK,
};

#endif