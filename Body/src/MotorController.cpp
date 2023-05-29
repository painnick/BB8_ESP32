#include "MotorController.h"

MotorController::MotorController()
    : targetMoveMs(0), dir(MOTOR_DIRECTION::STOP) {}

MotorController::~MotorController() {}

void MotorController::init() {
  pinMode(PIN_MOTOR1, OUTPUT);
  pinMode(PIN_MOTOR2, OUTPUT);
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
  digitalWrite(PIN_MOTOR1, LOW);
  digitalWrite(PIN_MOTOR2, HIGH);
  ESP_LOGD(MOTOR_TAG, "Left");
}

void MotorController::internalRight() {
  dir = MOTOR_DIRECTION::RIGHT;
  digitalWrite(PIN_MOTOR1, HIGH);
  digitalWrite(PIN_MOTOR2, LOW);
  ESP_LOGD(MOTOR_TAG, "Right");
}

void MotorController::stop() {
  dir = MOTOR_DIRECTION::STOP;
  digitalWrite(PIN_MOTOR1, LOW);
  digitalWrite(PIN_MOTOR2, LOW);
  ESP_LOGD(MOTOR_TAG, "Stop");
}

MotorController motor1;