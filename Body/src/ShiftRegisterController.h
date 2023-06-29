#pragma once

#include <Arduino.h>
#include <CircularBuffer.h>

#define SR_TAG "SR"

#define PIN_DATA 21
#define PIN_LATCH 22
#define PIN_CLOCK 23

typedef struct {
  unsigned long endMs;
  byte val;
} SR_ACTION;

class ShiftRegisterController {
public:
  ShiftRegisterController(uint8_t data_pin, uint8_t latch_pin,
                          uint8_t clock_pin);
  void loop(unsigned long now, bool forceUpdate = false);
  void set(byte newVal);
  void on(int index);
  void off(int index);
  void only(int index);
  byte get();
  void append(SR_ACTION action);

  void warningMessage();
  void randomLight(boolean isOn);

private:
  uint8_t pin_data;
  uint8_t pin_latch;
  uint8_t pin_clock;

  byte value;
  bool changed;
  bool isRandom {false};
  unsigned long lastChecked;

  CircularBuffer<SR_ACTION, 10> actions;

  void internalSet(byte val);
};

extern ShiftRegisterController shiftRegister;
