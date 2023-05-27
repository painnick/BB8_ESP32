#include "MotorController.h"

MotorController::MotorController()
    : targetMoveMs(0), dir(MOTOR_DIRECTION::STOP) {}

MotorController::~MotorController() {}

void MotorController::init() {
  ledcSetup(CHANNEL_MOTOR1, 1000, 8); // 0~255
  ledcSetup(CHANNEL_MOTOR2, 1000, 8); // 0~255

  ledcAttachPin(PIN_MOTOR1, CHANNEL_MOTOR1);
  ledcAttachPin(PIN_MOTOR2, CHANNEL_MOTOR2);

  ledcWrite(CHANNEL_MOTOR1, 0);
  ledcWrite(CHANNEL_MOTOR2, 255);
}

void MotorController::left(unsigned long ms) {
  unsigned long now = millis();
  targetMoveMs = now + ms;
  internalLeft();
}

void MotorController::right(unsigned long ms) {
  unsigned long now = millis();
  targetMoveMs = now + ms;
  internalRight();
}

void MotorController::loop() {
  if (dir != MOTOR_DIRECTION::STOP) {
    unsigned long now = millis();
    if (targetMoveMs - now < 100) {
      stop();
    }
  }
}

void MotorController::internalLeft() {
  dir = MOTOR_DIRECTION::LEFT;
  ledcWrite(CHANNEL_MOTOR1, 0);
  ledcWrite(CHANNEL_MOTOR2, 255);
  ESP_LOGI(MOTOR_TAG, "Left");
}

void MotorController::internalRight() {
  dir = MOTOR_DIRECTION::RIGHT;
  ledcWrite(CHANNEL_MOTOR1, 255);
  ledcWrite(CHANNEL_MOTOR2, 0);
  ESP_LOGI(MOTOR_TAG, "Right");
}

void MotorController::stop() {
  dir = MOTOR_DIRECTION::STOP;
  ledcWrite(CHANNEL_MOTOR1, 0);
  ledcWrite(CHANNEL_MOTOR2, 0);
  ESP_LOGI(MOTOR_TAG, "Stop");
}

MotorController motor1;