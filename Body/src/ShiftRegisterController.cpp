#include "ShiftRegisterController.h"

#include "esp_log.h"

ShiftRegisterController shiftRegister(PIN_DATA, PIN_LATCH, PIN_CLOCK);

ShiftRegisterController::ShiftRegisterController(uint8_t data_pin,
                                                 uint8_t latch_pin,
                                                 uint8_t clock_pin)
    : pin_data(data_pin), pin_latch(latch_pin), pin_clock(clock_pin), value(0),
      changed(false) {
  pinMode(pin_data, OUTPUT);
  pinMode(pin_latch, OUTPUT);
  pinMode(pin_clock, OUTPUT);
}

void ShiftRegisterController::update(bool forced) {
  if (forced || changed) {
    ESP_LOGD(SR_TAG, "UPDATE %d", value);
    digitalWrite(pin_latch, LOW);
    shiftOut(pin_data, pin_clock, LSBFIRST, value);
    digitalWrite(pin_latch, HIGH);
  }
  changed = false;
}

void ShiftRegisterController::set(byte newVal) {
  if (value != newVal) {
    value = newVal;
    ESP_LOGD(SR_TAG, "SR %d", value);
    changed = true;
  } else {
    changed = false;
  }
}

void ShiftRegisterController::on(int index) {
  byte newVal = value;
  bitSet(newVal, index);
  if (value != newVal) {
    value = newVal;
    ESP_LOGD(SR_TAG, "SR %d", value);
    changed = true;
  } else {
    changed = false;
  }
}

void ShiftRegisterController::off(int index) {
  byte newVal = value;
  bitClear(newVal, index);
  if (value != newVal) {
    value = newVal;
    ESP_LOGD(SR_TAG, "SR %d", value);
    changed = true;
  } else {
    changed = false;
  }
}

void ShiftRegisterController::only(int index) {
  byte newVal = 0;
  bitSet(newVal, index);
  if (value != newVal) {
    value = newVal;
    ESP_LOGD(SR_TAG, "SR %d", value);
    changed = true;
  } else {
    changed = false;
  }
}

byte ShiftRegisterController::get() { return value; }
