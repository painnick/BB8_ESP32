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
  if (actions.isEmpty()) {
    if (forced || changed) {
      internalSet(value);
    }
    changed = false;
  } else {
    unsigned long now = millis();
    SR_ACTION lastestAction = actions.first();
    if (lastestAction.endMs > now) {
      ESP_LOGD(SR_TAG, "Shift First Action");
      actions.shift();

      if (!actions.isEmpty()) {
        ESP_LOGD(SR_TAG, "Next Action");
        SR_ACTION newAction = actions.first();
        internalSet(newAction.val);
      }
    }
  }
}

void ShiftRegisterController::set(byte newVal) {
  actions.clear();

  if (value != newVal) {
    value = newVal;
    changed = true;
  } else {
    changed = false;
  }
}

void ShiftRegisterController::on(int index) {
  actions.clear();

  byte newVal = value;
  bitSet(newVal, index);
  if (value != newVal) {
    value = newVal;
    changed = true;
  } else {
    changed = false;
  }
}

void ShiftRegisterController::off(int index) {
  actions.clear();

  byte newVal = value;
  bitClear(newVal, index);
  if (value != newVal) {
    value = newVal;
    changed = true;
  } else {
    changed = false;
  }
}

void ShiftRegisterController::only(int index) {
  actions.clear();

  byte newVal = 0;
  bitSet(newVal, index);
  if (value != newVal) {
    value = newVal;
    changed = true;
  } else {
    changed = false;
  }
}

void ShiftRegisterController::append(SR_ACTION action) {
  if (actions.isEmpty()) {
    ESP_LOGD(SR_TAG, "Add First Action! %ul %02X", action.endMs, action.val);
    actions.push(action);
    internalSet(action.val);
  } else if (actions.isFull()) {
    ESP_LOGW(SR_TAG, "Action queue is FULL!");
  } else {
    SR_ACTION lastAction = actions.last();
    if (lastAction.endMs >= action.endMs) {
      ESP_LOGW(SR_TAG, "The previous request ends later!");
    } else {
      actions.push(action);
      ESP_LOGD(SR_TAG, "Add new Action %ul %02x", action.endMs, action.val);
    }
  }
}

byte ShiftRegisterController::get() { return value; }

void ShiftRegisterController::internalSet(byte val) {
  digitalWrite(pin_latch, LOW);
  shiftOut(pin_data, pin_clock, LSBFIRST, val);
  digitalWrite(pin_latch, HIGH);
  ESP_LOGD(SR_TAG, "SR %02X", value);
}
