#ifndef DISPLAY_H
#define DISPLAY_H 1

#define DISPLAY_BLANK 0x0F
#define DISPLAY_DEGREES 0x0E
#define DISPLAY_PERCENT 0x0F

#define DIGITS_OUTPUT_MASK 0b00110100
#define DOTS_SELECTION_INDEX 6
// NOTE: Represents how to output a given selection by clearing some bits.
// Assumes all the relevant bits are already HIGH.
const uint32 DigitsClearMask[] = {
  (~0b00000000) & DIGITS_OUTPUT_MASK,
  (~0b00000100) & DIGITS_OUTPUT_MASK,
  (~0b00010000) & DIGITS_OUTPUT_MASK,
  (~0b00010100) & DIGITS_OUTPUT_MASK,
  (~0b00100000) & DIGITS_OUTPUT_MASK,
  (~0b00100100) & DIGITS_OUTPUT_MASK,
  (~0b00110000) & DIGITS_OUTPUT_MASK,
  (~0b00110100) & DIGITS_OUTPUT_MASK,
};

enum DotsState {
  DOUBLE_COLUMNS = 0x0F,
  DOUBLE_PERIODS = 0x03,
  SINGLE_PERIOD  = 0x01
};

class Display {
  uint8 digits[6];
  DotsState dotsState;
  bool isBlinking;

  bool isDirty;

  void outputDigit(uint8 digit, uint selection);

  public:
  void setup();
  void tick(uint32 millis);
  void setDigits(uint8 d[6]);
  void setDotsState(DotsState state, bool blinking);
};

#endif