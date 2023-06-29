#include "LighterController.h"
#include "esp_log.h"

#define LIGHTER_TAG "LGT"

#define CHECK_INTERVAL_MS 50
#define PWM_FREQUENCY 1000
#define PWM_RESOLUTION 8

LighterController::LighterController(uint8_t pinNo)
    : pinNo(pinNo) {
  ledcSetup(ledChannel, PWM_FREQUENCY, PWM_RESOLUTION);
  ledcAttachPin(pinNo, ledChannel);
}

void LighterController::on() {
  isOn = true;
}

void LighterController::off() {
  isOn = false;
  ledcWrite(ledChannel, 0);
}

void LighterController::loop(unsigned long now) {
  if (isOn) {
    if (now - lastChecked > CHECK_INTERVAL_MS) {
      int bright = random(127, 255);
      ledcWrite(ledChannel, bright);
      lastChecked = now;
    }
  }
}

LighterController lighterController(PIN_LIGHTER);
