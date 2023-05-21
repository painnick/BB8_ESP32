#include <Arduino.h>

#include "esp_log.h"

#include "soc/rtc_cntl_reg.h" // disable brownout problems
#include "soc/soc.h"          // disable brownout problems

#include "ShiftRegisterController.h"

#include "VoiceRecognitionV3.h"

#define MAIN_TAG "Main"

// These are all GPIO pins on the ESP32
// Recommended pins include 2,4,12-19,21-23,25-27,32-33
// for the ESP32-S2 the GPIO pins are 1-21,26,33-42

#define PIN_DATA 23
#define PIN_LATCH 22
#define PIN_CLOCK 21

#define PIN_RX 15
#define PIN_TX 14

#define PIN_FLASH_LED 4
#define PIN_INTERNAL_LED 33

#define PIN_MOTOR1 18
#define PIN_MOTOR2 19

#define CHANNEL_MOTOR1 14
#define CHANNEL_MOTOR2 15

ShiftRegisterController controller(PIN_DATA, PIN_LATCH, PIN_CLOCK);

VR myVR;
uint8_t buf[255];
uint8_t records[7];

#define cmdSerial Serial1

#define COMMAND_DELIMETER "\r\n"
#define COMMAND_DELIMETER_SIZE 2
#define MAX_COMMAND_BUFFER_SZIE 50

String cmdBuffer = "";

void setup() {
  myVR.begin(9600);
  delay(500);
  records[0] = 0;
  records[1] = 1;
  records[2] = 2;
  records[3] = 3;
  int ret = myVR.setAutoLoad(records, 4);
  ESP_LOGI(MAIN_TAG, "Setup VoiceRecognition");

  cmdSerial.begin(115200, SERIAL_8N1, PIN_RX, PIN_TX);
  delay(500);
  ESP_LOGI(MAIN_TAG, "Setup Command-Serial");

  pinMode(PIN_INTERNAL_LED, OUTPUT);
  controller.set(0);
  controller.update();

  ledcSetup(CHANNEL_MOTOR1, 1000, 8); // 0~255
  ledcSetup(CHANNEL_MOTOR2, 1000, 8); // 0~255

  ledcAttachPin(PIN_MOTOR1, CHANNEL_MOTOR1);
  ledcAttachPin(PIN_MOTOR2, CHANNEL_MOTOR2);

  ledcWrite(CHANNEL_MOTOR1, 0);
  ledcWrite(CHANNEL_MOTOR2, 255);

  ESP_LOGI(MAIN_TAG, "Setup Body");

  cmdSerial.printf("NOP\r\n");
  cmdSerial.printf("NOP\r\n");
}

void loop() {

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

        byte curVal = 0;
        controller.set(bitSet(curVal, 6));
        // delay(10);
      }
    }
  } else {
    controller.set(0);
  }

  // Check VoiceRecognition
  int ret = myVR.recognize(buf, 50);
  if (ret > 0) {
    int cmd = buf[2];
    cmdSerial.printf("LED%d\r\n", cmd);
    cmdSerial.flush();
    ESP_LOGI(MAIN_TAG, "=> LED%d", cmd);

    ESP_LOGI(MAIN_TAG, "!!!Command!!! %d", cmd);
  }

  controller.update();
}
