#pragma once

#include <Arduino.h>

#define PIN_LIGHTER 14

class LighterController {
public:
  LighterController(uint8_t pinNo);
  void on();
  void off();
  void loop(unsigned long now);

private:
  uint8_t pinNo;
  const uint8_t ledChannel{7};
  boolean isOn{false};
  unsigned int tick{};
  unsigned int lastChecked{};
};

extern LighterController lighterController;
