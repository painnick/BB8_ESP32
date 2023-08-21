#include "MonoEyeController.h"

#define CHECK_INTERVAL_MS 1000
#define PROBABILITY 5

MonoEyeController::MonoEyeController(uint8_t pinNo)
        : pinNo(pinNo), isWaiting(true) {
    pinMode(pinNo, OUTPUT);
}

void MonoEyeController::wakeUp() {
    isWaiting = false;
    digitalWrite(pinNo, HIGH);
}

void MonoEyeController::sleep() {
    isWaiting = true;
    digitalWrite(pinNo, LOW);
}

void MonoEyeController::loop(unsigned long now) {
    if (now - lastChecked > CHECK_INTERVAL_MS) {
        tick = (++tick) % PROBABILITY;
        bool isOn = isWaiting ? (tick == 0) : (tick != 0);
        if (isOn) {
            digitalWrite(pinNo, HIGH);
        } else {
            digitalWrite(pinNo, LOW);
        }
        lastChecked = now;
    }
}

MonoEyeController monoEyeController(PIN_MONO_EYE);
