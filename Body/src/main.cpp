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

void setup() {

  ESP_LOGI(MAIN_TAG, "Setup...");

  setupSound();

  vr.init([](int cmd) -> void {
    switch (cmd) {
    case 0: // HELLO
      dfmp3.playRandomTrackFromAll();
      break;
    case 2: // RIGHT
      motor1.right(1000 * 3);
      break;
    case 3: // LEFT
      motor1.left(1000 * 3);
      break;
    case 4: // STOP
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
  });

  ESP_LOGI(MAIN_TAG, "Setup Body");
}

void loop() {
  motor1.loop();
  commander1.loop();
  shiftRegister.update();
  vr.loop();
  dfmp3.loop();
}
