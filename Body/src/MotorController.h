#pragma once

#include <Arduino.h>

#include "esp_log.h"

#define MOTOR_TAG "MOTOR"

#define PIN_MOTOR1 18
#define PIN_MOTOR2 19

#define CHANNEL_MOTOR1 14
#define CHANNEL_MOTOR2 15

enum class MOTOR_DIRECTION {
  STOP = 0,
  LEFT = 1,
  RIGHT = 2,
};

class MotorController {
public:
  MotorController();
  ~MotorController();
  void init();
  void left(unsigned long ms);
  void right(unsigned long ms);
  void stop();
  void loop();

private:
  unsigned long targetMoveMs;
  MOTOR_DIRECTION dir;

  void internalLeft();
  void internalRight();
};

extern MotorController motor1;
