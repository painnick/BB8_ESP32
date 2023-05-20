#include "ShiftRegisterController.h"

ShiftRegisterController::ShiftRegisterController(uint8_t data_pin,
                                                 uint8_t latch_pin,
                                                 uint8_t clock_pin)
    : PIN_DATA(data_pin), PIN_LATCH(latch_pin), PIN_CLOCK(clock_pin), value(0),
      changed(false) {
  pinMode(PIN_DATA, OUTPUT);
  pinMode(PIN_LATCH, OUTPUT);
  pinMode(PIN_CLOCK, OUTPUT);
}

void ShiftRegisterController::update() {
  if (changed) {
    digitalWrite(PIN_LATCH, LOW);
    shiftOut(PIN_DATA, PIN_CLOCK, LSBFIRST, value);
    digitalWrite(PIN_LATCH, HIGH);
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
