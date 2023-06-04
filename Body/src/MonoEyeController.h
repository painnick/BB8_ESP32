#pragma once

#include <Arduino.h>

#define PIN_MONO_EYE 32

class MonoEyeController {
public:
  MonoEyeController(uint8_t pinNo);
  void wakeUp();
  void sleep();
  void loop(unsigned long now);

private:
  uint8_t pinNo;
  bool isWaiting;
  unsigned int tick{};
  unsigned int lastChecked{};
};

extern MonoEyeController monoEyeController;
