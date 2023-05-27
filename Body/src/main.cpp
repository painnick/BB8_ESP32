#include <Arduino.h>

#include "esp_log.h"

#define MAIN_TAG "Main"

// These are all GPIO pins on the ESP32
// Recommended pins include 2,4,12-19,21-23,25-27,32-33
// for the ESP32-S2 the GPIO pins are 1-21,26,33-42

#define PIN_RX 15
#define PIN_TX 14

#define PIN_FLASH_LED 4
#define PIN_INTERNAL_LED 33

#define PIN_MOTOR1 18
#define PIN_MOTOR2 19

#define CHANNEL_MOTOR1 14
#define CHANNEL_MOTOR2 15

#define USE_SOUND
// #define USE_COMMANDER
#define USE_VR
#define USE_SHIFT_REGISTER

#ifdef USE_COMMANDER
#include "SoftwareSerial.h"
#endif

#ifdef USE_VR
#include "VoiceRecognitionController.h"
#endif

#ifdef USE_SHIFT_REGISTER
#include "ShiftRegisterController.h"
#endif

#ifdef USE_SOUND
#include "Mp3Controller.h"
#endif

#ifdef USE_COMMANDER
#define cmdSerial Serial1

#define COMMAND_DELIMETER "\r\n"
#define COMMAND_DELIMETER_SIZE 2
#define MAX_COMMAND_BUFFER_SZIE 50

String cmdBuffer = "";
#endif

#ifdef USE_COMMANDER
void setupCommander() {
  cmdSerial.begin(115200, SERIAL_8N1, PIN_RX, PIN_TX);
  delay(500);
  ESP_LOGI(MAIN_TAG, "Setup Command-Serial");
}
#endif

void setupMotor() {
  ledcSetup(CHANNEL_MOTOR1, 1000, 8); // 0~255
  ledcSetup(CHANNEL_MOTOR2, 1000, 8); // 0~255

  ledcAttachPin(PIN_MOTOR1, CHANNEL_MOTOR1);
  ledcAttachPin(PIN_MOTOR2, CHANNEL_MOTOR2);

  ledcWrite(CHANNEL_MOTOR1, 0);
  ledcWrite(CHANNEL_MOTOR2, 255);
}

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

#ifdef USE_COMMANDER
  // Check Command
  if (cmdSerial.available()) {
    // Append command-buffer
    while (cmdSerial.available()) {
      cmdBuffer += (char)cmdSerial.read();
    }
    // Check size of command-buffer
    if (cmdBuffer.length() > MAX_COMMAND_BUFFER_SZIE) {
      cmdBuffer = "";
    } else {
      int found = cmdBuffer.indexOf(COMMAND_DELIMETER);
      if (found != -1) {
        String cmd = cmdBuffer.substring(0, found);
        cmdBuffer = cmdBuffer.substring(found + COMMAND_DELIMETER_SIZE);
        ESP_LOGI(MAIN_TAG, "<= %s", cmd.c_str());

        if (cmd == "PLAY1") {
          int newSong = 1;
          if (song != newSong) {
#ifdef USE_SOUND
            dfmp3.nextTrack();
#endif
            song = newSong;
          }
        } else if (cmd == "PLAY2") {
          int newSong = 0;
          if (song != newSong) {
#ifdef USE_SOUND
            dfmp3.stop();
#endif
            song = newSong;
          }
        }

        byte curVal = 0;
        shiftRegister.set(bitSet(curVal, 6));
        // delay(10);
      }
    }
  } else {
    shiftRegister.set(0);
  }
#endif

#ifdef USE_VR
  // Check VoiceRecognition
  int ret = myVR.recognize(buf, 50);
  if (ret > 0) {
    ESP_LOGI(MAIN_TAG, "VR %d", ret);
#ifdef USE_COMMANDER
    int cmd = buf[2];
    cmdSerial.printf("LED%d\r\n", cmd);
    cmdSerial.flush();
    ESP_LOGI(MAIN_TAG, "=> LED%d", cmd);

    ESP_LOGI(MAIN_TAG, "!!!Command!!! %d", cmd);
#endif
  }
#endif

#ifdef USE_SHIFT_REGISTER
  shiftRegister.update();
#endif

#ifdef USE_SOUND
  dfmp3.loop();
#endif
}
