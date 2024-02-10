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
  BLANK          = 0x00,
  DOUBLE_COLUMNS = 0x0F,
  DOUBLE_PERIODS = 0x03,
  SINGLE_PERIOD  = 0x01
};

struct DisplayValue {
  uint8 digits[6];

  DisplayValue() {}

  DisplayValue(uint8 d[6]) {
    for(uint i = 0; i < 6; i++) {
      this->digits[i] = d[i];
    }
  }
};

class Display {
  uint32 transitionInitialMillis;
  
  bool isInTransition;
  bool hasNewValue;

  DotsState dotsState;
  
  DisplayValue oldValue;
  DisplayValue currentValue;

  public:
  void setup();
  void tick(uint32 millis);
  void setDigits(uint8 d[6]);
  void setDotsState(DotsState state);
};

#endif