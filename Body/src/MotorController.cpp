#include "MotorController.h"

#include <utility>

MotorController::MotorController() : startMoveMs(0), endMoveMs(0), dir(MOTOR_DIRECTION::STOP) {}

MotorController::~MotorController() = default;

void MotorController::init() {
    pinMode(PIN_MOTOR1, OUTPUT);
    pinMode(PIN_MOTOR2, OUTPUT);
}

void MotorController::left(unsigned long ms, MotorCallback cb,
                           unsigned long startDelayMs) {
    callback = std::move(cb);
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

void MotorController::right(unsigned long ms, MotorCallback cb,
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

void MotorController::loop(unsigned long now) {
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
        if (endMoveMs - now < 100) {
            MotorCallback lastCallback = callback;
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
#ifdef DEBUG
    ESP_LOGD(MOTOR_TAG, "Left");
#endif
}

void MotorController::internalRight() {
    dir = MOTOR_DIRECTION::RIGHT;
    digitalWrite(PIN_MOTOR1, LOW);
    digitalWrite(PIN_MOTOR2, HIGH);
#ifdef DEBUG
    ESP_LOGD(MOTOR_TAG, "Right");
#endif
}

void MotorController::stop() {
    dir = MOTOR_DIRECTION::STOP;
    digitalWrite(PIN_MOTOR1, LOW);
    digitalWrite(PIN_MOTOR2, LOW);
#ifdef DEBUG
    ESP_LOGD(MOTOR_TAG, "Stop");
#endif
}

MotorController motor1;