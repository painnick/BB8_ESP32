#include <Arduino.h>

#include "esp_log.h"

#define MAIN_TAG "Main"

// These are all GPIO pins on the ESP32
// Recommended pins include 2,4,12-19,21-23,25-27,32-33
// for the ESP32-S2 the GPIO pins are 1-21,26,33-42

#define PIN_FLASH_LED 4
#define PIN_INTERNAL_LED 33

#define USE_COMMANDER
#define USE_VR

#include "Commander.h"
#include "MotorController.h"
#include "Mp3Controller.h"
#include "ShiftRegisterController.h"
#include "SoftwareSerial.h"
#include "VoiceRecognitionController.h"

void randomMoveMotor(unsigned long duration, MotorCallnack callback = nullptr,
                     unsigned long startDelayMs = 0) {
  if ((random(1024) % 2) == 0)
    motor1.left(duration, callback, startDelayMs);
  else
    motor1.right(duration, callback, startDelayMs);
}

void moveMotorOpposite(MotorController *mc, MOTOR_DIRECTION dir,
                       unsigned long duration) {
  if (dir == MOTOR_DIRECTION::LEFT) {
    mc->right(duration);
  } else if (dir == MOTOR_DIRECTION::RIGHT) {
    mc->left(duration);
  }
}

int lastCommand = -1;
int duplicateCommandCount = 0;
void setup() {

  ESP_LOGI(MAIN_TAG, "Setup...");

  setupSound();

  // commander1.send("NOP");

  vr.init([](int cmd) -> void {
    if (cmd < 0) {
      return;
    }

    if (lastCommand == cmd) {
      ++duplicateCommandCount;
    } else {
      duplicateCommandCount = 1;
    }
    lastCommand = cmd;

    if (duplicateCommandCount > 2) {
      ESP_LOGD(MAIN_TAG, "Same command %dtimes", duplicateCommandCount);
      playWhy();
      randomMoveMotor(
          1000,
          [](MotorController *mc, MOTOR_DIRECTION dir) -> void {
            moveMotorOpposite(mc, dir, 500);
          },
          1000);
      unsigned long now = millis();
      shiftRegister.append({.endMs = now + 300, .val = 0xFF});
      shiftRegister.append({.endMs = now + 600, .val = 0x00});
      shiftRegister.append({.endMs = now + 900, .val = 0xFF});
      shiftRegister.append({.endMs = now + 1200, .val = 0x00});
      return;
    }

    if (random(32) == 3) {
      playFail();
      randomMoveMotor(600, nullptr, 1000);
      return;
    }

    switch (cmd) {
    case 0: // HELLO
      randomPlayGeneral();
      commander1.send("LED1ON");
      commander1.send("LED2ON");
      commander1.send("LED3ON");
      commander1.send("WIFION");
      shiftRegister.set(0xFF);
      break;
    case 1: // BYE
      playBye();
      motor1.stop();
      commander1.send("LED1OFF");
      commander1.send("LED2OFF");
      commander1.send("LED3OFF");
      commander1.send("WIFIOFF");
      shiftRegister.set(0);
      break;
    case 2: // RIGHT
      motor1.right(500);
      break;
    case 3: // LEFT
      motor1.left(500);
      break;
    case 4: // STOP
      dfmp3.stop();
      motor1.stop();
      break;
    default:
      dfmp3.stop();
      break;
    }
  });

  motor1.init();

  commander1.init([](const Commander *, const String &cmd) -> void {
    ESP_LOGD(MAIN_TAG, "<= Recv : %s", cmd.c_str());
    if (cmd == "Left") {
      motor1.left(300);
    } else if (cmd == "Right") {
      motor1.right(300);
    }
  });

  unsigned long now = millis();
  randomSeed(now);
  randomMoveMotor(
      100 * random(3, 7),
      [](MotorController *mc, MOTOR_DIRECTION dir) -> void {
        moveMotorOpposite(mc, dir, 500);
      },
      1000);

  randomPlayGeneral();

  // Head의 기존 상태 값을 초기화
  commander1.send("LED1OFF");
  commander1.send("LED2OFF");
  commander1.send("LED3OFF");
  commander1.send("WIFIOFF");

  unsigned long now1 = millis();
  shiftRegister.append({.endMs = now1 + 200, .val = 0xFF});
  shiftRegister.append({.endMs = now1 + 400, .val = 0x00});
  shiftRegister.append({.endMs = now1 + 600, .val = 0xFF});
  shiftRegister.append({.endMs = now1 + 800, .val = 0x00});

  ESP_LOGI(MAIN_TAG, "Setup Body");
}

void loop() {
  motor1.loop();
  commander1.loop();
  shiftRegister.update();
  vr.loop();
  dfmp3.loop();
}
