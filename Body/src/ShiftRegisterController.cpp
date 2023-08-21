#include "ShiftRegisterController.h"

#include "esp_log.h"

#define RANDOM_INTERVAL_MS 300

ShiftRegisterController shiftRegister(PIN_DATA, PIN_LATCH, PIN_CLOCK);

ShiftRegisterController::ShiftRegisterController(uint8_t data_pin,
                                                 uint8_t latch_pin,
                                                 uint8_t clock_pin)
        : pin_data(data_pin),
          pin_latch(latch_pin),
          pin_clock(clock_pin),
          value(0),
          changed(false) {
    pinMode(pin_data, OUTPUT);
    pinMode(pin_latch, OUTPUT);
    pinMode(pin_clock, OUTPUT);
}

void ShiftRegisterController::loop(unsigned long now, bool forceUpdate) {
    switch (mode) {
        case ShiftRegisterMode::FIXED:
            if (forceUpdate || changed) {
                internalSet(value);
                changed = false;
            }
            break;
        case ShiftRegisterMode::ACTIONS:
            if (actions.isEmpty()) {
                if (forceUpdate || changed) {
                    internalSet(value);
                }
                changed = false;
            } else {
                SR_ACTION lastestAction = actions.first();
                if (lastestAction.endMs > now) {
#ifdef DEBUG
                    ESP_LOGD(SR_TAG, "Shift First Action");
#endif
                    actions.shift();

                    if (!actions.isEmpty()) {
#ifdef DEBUG
                        ESP_LOGD(SR_TAG, "Next Action");
#endif
                        SR_ACTION newAction = actions.first();
                        internalSet(newAction.val);
                    }
                }
            }
            break;
        case ShiftRegisterMode::RANDOM:
            if (now - lastChecked > RANDOM_INTERVAL_MS) {
                internalSet(random(256));
                lastChecked = now;
            }
            break;
        default:
            break;
    }
}

void ShiftRegisterController::set(byte newVal) {
    actions.clear();

    mode = ShiftRegisterMode::FIXED;

    if (value != newVal) {
        value = newVal;
        changed = true;
    } else {
        changed = false;
    }
}

void ShiftRegisterController::on(int index) {
    actions.clear();

    mode = ShiftRegisterMode::FIXED;

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

    mode = ShiftRegisterMode::FIXED;

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

    mode = ShiftRegisterMode::FIXED;

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
    mode = ShiftRegisterMode::ACTIONS;

    if (actions.isEmpty()) {
#ifdef DEBUG
        ESP_LOGD(SR_TAG, "Add First Action! %ul %02X", action.endMs, action.val);
#endif
        actions.push(action);
        internalSet(action.val);
    } else if (actions.isFull()) {
#ifdef DEBUG
        ESP_LOGW(SR_TAG, "Action queue is FULL!");
#endif
    } else {
        SR_ACTION lastAction = actions.last();
        if (lastAction.endMs >= action.endMs) {
#ifdef DEBUG
            ESP_LOGW(SR_TAG, "The previous request ends later!");
#endif
        } else {
            actions.push(action);
#ifdef DEBUG
            ESP_LOGD(SR_TAG, "Add new Action %ul %02x", action.endMs, action.val);
#endif
        }
    }
}

byte ShiftRegisterController::get() { return value; }

void ShiftRegisterController::internalSet(byte val) {
    digitalWrite(pin_latch, LOW);
    shiftOut(pin_data, pin_clock, LSBFIRST, val);
    digitalWrite(pin_latch, HIGH);
#ifdef DEBUG
    ESP_LOGD(SR_TAG, "%02X", val);
#endif
}

void ShiftRegisterController::warningMessage() {
    unsigned long now = millis();

    mode = ShiftRegisterMode::ACTIONS;

    append({.endMs = now + 300, .val = 0xFF});
    append({.endMs = now + 600, .val = 0x00});
    append({.endMs = now + 900, .val = 0xFF});
    append({.endMs = now + 1200, .val = 0x00});
}

void ShiftRegisterController::randomLight(boolean isOn) {
    actions.clear();

    if (isOn) {
        lastMode = mode;
        lastValue = value;
        mode = ShiftRegisterMode::RANDOM;
    } else {
        if (mode == ShiftRegisterMode::RANDOM) {
            mode = lastMode;
            value = lastValue;
            changed = true;
        }
    }
}
