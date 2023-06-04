#include "MotorController.h"

MotorController::MotorController() : endMoveMs(0), dir(MOTOR_DIRECTION::STOP) {}

MotorController::~MotorController() {}

void MotorController::init() {
  pinMode(PIN_MOTOR1, OUTPUT);
  pinMode(PIN_MOTOR2, OUTPUT);
}

void MotorController::left(unsigned long ms, MotorCallnack cb,
                           unsigned long startDelayMs) {
  callback = cb;
  unsigned long now = millis();
  endMoveMs = now + ms + startDelayMs;
  if (startDelayMs == 0) {
    startMoveMs = 0;
    internalLeft();
  } else {
    dir = MOTOR_DIRECTION::LEFT;
    startMoveMs = now + startDelayMs;
  }
}

void MotorController::right(unsigned long ms, MotorCallnack cb,
                            unsigned long startDelayMs) {
  callback = cb;
  unsigned long now = millis();
  endMoveMs = now + ms + startDelayMs;
  if (startDelayMs == 0) {
    startMoveMs = 0;
    internalRight();
  } else {
    dir = MOTOR_DIRECTION::RIGHT;
    startMoveMs = now + startDelayMs;
  }
}

void MotorController::loop() {
  unsigned long now = millis();
  if (startMoveMs != 0) {
    if (now > startMoveMs) {
      startMoveMs = 0;
      if (dir == MOTOR_DIRECTION::LEFT) {
        internalLeft();
      } else if (dir == MOTOR_DIRECTION::RIGHT) {
        internalRight();
      }
    }
  }

  if (dir != MOTOR_DIRECTION::STOP) {
    unsigned long now = millis();
    if (endMoveMs - now < 100) {
      MotorCallnack lastCallback = callback;
      MOTOR_DIRECTION lastDir = dir;
      stop();
      if (lastCallback != nullptr) {
        lastCallback(this, lastDir);
      }
    }
  }
}

void MotorController::internalLeft() {
  dir = MOTOR_DIRECTION::LEFT;
  digitalWrite(PIN_MOTOR1, HIGH);
  digitalWrite(PIN_MOTOR2, LOW);
  ESP_LOGD(MOTOR_TAG, "Left");
}

void MotorController::internalRight() {
  dir = MOTOR_DIRECTION::RIGHT;
  digitalWrite(PIN_MOTOR1, LOW);
  digitalWrite(PIN_MOTOR2, HIGH);
  ESP_LOGD(MOTOR_TAG, "Right");
}

void MotorController::stop() {
  dir = MOTOR_DIRECTION::STOP;
  digitalWrite(PIN_MOTOR1, LOW);
  digitalWrite(PIN_MOTOR2, LOW);
  ESP_LOGD(MOTOR_TAG, "Stop");
}

MotorController motor1;