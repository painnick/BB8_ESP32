#pragma once

#include <Arduino.h>

#define SR_TAG "SR"

class ShiftRegisterController {
public:
  ShiftRegisterController(uint8_t data_pin, uint8_t latch_pin,
                          uint8_t clock_pin);
  void update();
  void set(byte newVal);
  void on(int index);
  void off(int index);
  void only(int index);
  byte get();

private:
  uint8_t pin_data;
  uint8_t pin_latch;
  uint8_t pin_clock;

  byte value;
  bool changed;
};
