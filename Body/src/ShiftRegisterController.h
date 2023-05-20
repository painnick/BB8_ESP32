#pragma once

#include <Arduino.h>

#define SR_TAG "SR"

class ShiftRegisterController {
public:
  ShiftRegisterController(uint8_t data_pin, uint8_t latch_pin,
                          uint8_t clock_pin);
  void update();
  void set(byte newVal);
  byte get();

private:
  uint8_t PIN_DATA;
  uint8_t PIN_LATCH;
  uint8_t PIN_CLOCK;

  byte value;
  bool changed;
};
