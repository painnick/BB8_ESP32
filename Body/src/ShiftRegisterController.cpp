#include "ShiftRegisterController.h"

ShiftRegisterController::ShiftRegisterController(uint8_t data_pin,
                                                 uint8_t latch_pin,
                                                 uint8_t clock_pin)
    : pin_data(data_pin), pin_latch(latch_pin), pin_clock(clock_pin), value(0),
      changed(false) {
  pinMode(pin_data, OUTPUT);
  pinMode(pin_latch, OUTPUT);
  pinMode(pin_clock, OUTPUT);
}

void ShiftRegisterController::update() {
  if (changed) {
    digitalWrite(pin_latch, LOW);
    shiftOut(pin_data, pin_clock, LSBFIRST, value);
    digitalWrite(pin_latch, HIGH);
  }
}

void ShiftRegisterController::set(byte newVal) {
  if (value != newVal) {
    value = newVal;
    changed = true;
  } else {
    changed = false;
  }
}

byte ShiftRegisterController::get() { return value; }
