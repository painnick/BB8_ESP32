#include <Arduino.h>

#include "esp_log.h"

#define MAIN_TAG "Main"

// These are all GPIO pins on the ESP32
// Recommended pins include 2,4,12-19,21-23,25-27,32-33
// for the ESP32-S2 the GPIO pins are 1-21,26,33-42

#define PIN_FLASH_LED 4
#define PIN_INTERNAL_LED 33

#define COMMAND_REPEATATION_COUNT 5

#ifdef DEBUG
#define SAY_ANYTHING_INTERVAL_SEC 30
#else
#define SAY_ANYTHING_INTERVAL_SEC 60
#endif

#include "Commander.h"
#include "MonoEyeController.h"
#include "MotorController.h"
#include "Mp3Controller.h"
#include "ShiftRegisterController.h"
#include "SoftwareSerial.h"
#include "VoiceRecognitionController.h"

void randomMoveMotor(unsigned long duration, MotorCallback callback = nullptr,
                     unsigned long startDelayMs = 0) {
  if ((random(1024) % 2) == 0)
    motor1.left(duration, callback, startDelayMs);
  else
    motor1.right(duration, callback, startDelayMs);
}

void moveMotorOpposite(MotorController *mc, MOTOR_DIRECTION dir,
                       unsigned long duration,
                       MotorCallback callback = nullptr) {
  if (dir == MOTOR_DIRECTION::LEFT) {
    mc->right(duration, callback);
  } else if (dir == MOTOR_DIRECTION::RIGHT) {
    mc->left(duration, callback);
  }
}

bool isWaiting = true;
bool isMonoEyeOn = false;

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

    if (duplicateCommandCount >= COMMAND_REPEATATION_COUNT) {
      ESP_LOGD(MAIN_TAG, "Same command %dtimes", duplicateCommandCount);
      playWhy();
      randomMoveMotor(
          1500,
          [](MotorController *mc, MOTOR_DIRECTION dir) -> void {
            moveMotorOpposite(mc, dir, 500);
          },
          1000);
      shiftRegister.warningMessage();
      duplicateCommandCount = 0;
      return;
    }

    if (random(32) == 3) {
      playFail();
      randomMoveMotor(600, nullptr, 1000);
      return;
    }

    switch (cmd) {
    case VR_HELLO: // HELLO
      monoEyeController.wakeUp();
      randomPlayGeneral();
      randomMoveMotor(
          200 + random(0, 5) * 100,
          [](MotorController *mc, MOTOR_DIRECTION dir) -> void {
            moveMotorOpposite(mc, dir, 200 + random(0, 5) * 100);
          },
          1000);
      commander1.send("LED1ON");
      commander1.send("LED2ON");
      commander1.send("LED3ON");
      commander1.send("WIFION");
      shiftRegister.set(0xFF);
      break;
    case VR_BYE: // BYE
      monoEyeController.sleep();
      playBye();
      motor1.stop();
      commander1.send("LED1OFF");
      commander1.send("LED2OFF");
      commander1.send("LED3OFF");
      commander1.send("WIFIOFF");
      shiftRegister.set(0);
      break;
    case VR_RIGHT: // RIGHT
      motor1.right(500);
      break;
    case VR_LEFT: // LEFT
      motor1.left(500);
      break;
    case VR_STOP: // STOP
      dfmp3.stop();
      motor1.stop();
      break;
    case VR_FOOL: // FOOL!
      playFail();
      shiftRegister.warningMessage();
      randomMoveMotor(
          500,
          [](MotorController *mc, MOTOR_DIRECTION dir) -> void {
            moveMotorOpposite(
                mc, dir, 500,
                [](MotorController *mc, MOTOR_DIRECTION dir) -> void {
                  moveMotorOpposite(
                      mc, dir, 300,
                      [](MotorController *mc, MOTOR_DIRECTION dir) -> void {
                        moveMotorOpposite(mc, dir, 300);
                      });
                });
          },
          500);
      break;
    case VR_MUSIC: // MUSIC
      motor1.stop();
      playOST();
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

  // unsigned long now = millis();
  // randomSeed(now);
  // randomMoveMotor(
  //     200 + random(0, 5) * 100,
  //     [](MotorController *mc, MOTOR_DIRECTION dir) -> void {
  //       moveMotorOpposite(mc, dir, 200 + random(0, 5) * 100);
  //     },
  //     1000);

  randomPlayGeneral();

  // Head의 기존 상태 값을 초기화
  commander1.send("LED1OFF");
  commander1.send("LED2OFF");
  commander1.send("LED3OFF");
  commander1.send("WIFIOFF");

  shiftRegister.warningMessage();

  ESP_LOGI(MAIN_TAG, "Setup Body");
}

unsigned long lastSaid = 0;
void loop() {
  unsigned long now = millis();
  monoEyeController.loop(now);
  if (now - lastSaid > 1000 * SAY_ANYTHING_INTERVAL_SEC) {
    randomPlayGeneral();
    lastSaid = now;
  }
  motor1.loop();
  commander1.loop();
  shiftRegister.update();
  vr.loop();
  dfmp3.loop();
}
