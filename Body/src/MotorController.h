#pragma once

#include <Arduino.h>

#include "esp_log.h"

#define MOTOR_TAG "MOTOR"

#define PIN_MOTOR1 18
#define PIN_MOTOR2 19

#define CHANNEL_MOTOR1 14
#define CHANNEL_MOTOR2 15

class MotorController {
public:
  MotorController();
  ~MotorController();
  void init();
};
