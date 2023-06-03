#pragma once

#include <Arduino.h>
#include <functional>

#include "esp_log.h"

#define MOTOR_TAG "MOTOR"

#define PIN_MOTOR1 18
#define PIN_MOTOR2 19

enum class MOTOR_DIRECTION {
  STOP = 0,
  LEFT = 1,
  RIGHT = 2,
};

class MotorController;
typedef std::function<void(MotorController *, MOTOR_DIRECTION)> MotorCallnack;

class MotorController {
public:
  MotorController();
  ~MotorController();
  void init();
  void left(unsigned long ms, MotorCallnack callback = nullptr,
            unsigned long startDelatMs = 0);
  void right(unsigned long ms, MotorCallnack callback = nullptr,
             unsigned long startDelatMs = 0);
  void stop();
  void loop();

private:
  unsigned long startMoveMs;
  unsigned long endMoveMs;
  MOTOR_DIRECTION dir;
  MotorCallnack callback;

  void internalLeft();
  void internalRight();
};

extern MotorController motor1;
