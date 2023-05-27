#include <Arduino.h>

#include "esp_log.h"

#define MAIN_TAG "Main"

// These are all GPIO pins on the ESP32
// Recommended pins include 2,4,12-19,21-23,25-27,32-33
// for the ESP32-S2 the GPIO pins are 1-21,26,33-42

#define PIN_FLASH_LED 4
#define PIN_INTERNAL_LED 33

#define USE_SOUND
#define USE_COMMANDER
#define USE_VR
#define USE_SHIFT_REGISTER

#include "Commander.h"
#include "MotorController.h"
#include "Mp3Controller.h"
#include "ShiftRegisterController.h"
#include "SoftwareSerial.h"
#include "VoiceRecognitionController.h"

void setup() {

  delay(1000);

#ifdef USE_VR
  setupVR();
#endif

#ifdef USE_SOUND
  setupSound();
#endif

#ifdef USE_COMMANDER
  setupCommander();
#endif

#ifdef USE_SHIFT_REGISTER
  setupShiftRegister();
#endif

  setupMotor();

  ESP_LOGI(MAIN_TAG, "Setup Body");
}

int song = 0;
// int srTemp1 = 0;
unsigned long lastChecked1 = 0;
void loop() {
  // unsigned long now = millis();
  // if (now - lastChecked1 > 1000 * 1) {
  //   srTemp1 = (++srTemp1) % 8;
  //   shiftRegister.only(srTemp1);
  //   lastChecked1 = now;
  // }

#ifdef USE_VR
  // Check VoiceRecognition
  int ret = myVR.recognize(buf, 50);
  if (ret > 0) {
    ESP_LOGI(MAIN_TAG, "VR %d", ret);
  }
#endif

#ifdef USE_SHIFT_REGISTER
  shiftRegister.update();
#endif

#ifdef USE_SOUND
  dfmp3.loop();
#endif
}
