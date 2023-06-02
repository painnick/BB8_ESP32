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

#define SOUND_WELCOME 1
#define SOUND_HELLO 2
#define SOUND_FAIL 3
#define SOUND_WHY 4
#define SOUND_SLEEP 5
#define SOUND_THEME 6

void setup() {

  ESP_LOGI(MAIN_TAG, "Setup...");

  setupSound();

  dfmp3.playMp3FolderTrack(SOUND_WELCOME);

  commander1.send("NOP");

  vr.init([](int cmd) -> void {
    switch (cmd) {
    case 0: // HELLO
      dfmp3.playMp3FolderTrack(SOUND_HELLO);
      commander1.send("LED1ON");
      commander1.send("LED2ON");
      commander1.send("LED3ON");

      shiftRegister.set(0xFF);
      break;
    case 1: // BYE
      dfmp3.playMp3FolderTrack(SOUND_SLEEP);
      motor1.stop();
      commander1.send("LED1OFF");
      commander1.send("LED2OFF");
      commander1.send("LED3OFF");

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
      motor1.left(80);
    } else if (cmd == "Right") {
      motor1.right(80);
    }
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
