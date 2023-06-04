#include "MonoEyeController.h"

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
  if (isWaiting) {
    if (now - lastChecked > 1000) {
      tick = (++tick) % 5;
      if (tick == 0) {
        digitalWrite(pinNo, HIGH);
      } else {
        digitalWrite(pinNo, LOW);
      }
      lastChecked = now;
    }
  }
}

MonoEyeController monoEyeController(PIN_MONO_EYE);
