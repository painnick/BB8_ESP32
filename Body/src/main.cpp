#include <Arduino.h>

#include "esp_log.h"

#include "soc/rtc_cntl_reg.h" // disable brownout problems
#include "soc/soc.h"          // disable brownout problems

#include <SoftwareSerial.h>

#include "ShiftRegisterController.h"

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

SoftwareSerial cmdSerial(PIN_RX, PIN_TX);

ShiftRegisterController controller(PIN_DATA, PIN_LATCH, PIN_CLOCK);

void setup() {
  cmdSerial.begin(9600);

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

unsigned long lastTime = 0;
int i = 0;
void loop() {
  i = (++i) % 8;
  unsigned long now = millis();
  if (now - lastTime > 3000) {
    lastTime = now;
    cmdSerial.printf("Command%d\r\n", i);
    cmdSerial.flush();
    ESP_LOGI(MAIN_TAG, "Send! %d", i);
  }

  if (cmdSerial.available()) {
    ESP_LOGI(MAIN_TAG, "=> %s", cmdSerial.readString());

    byte curVal = 0;
    controller.set(bitSet(curVal, 7));
  } else {
    controller.set(0);
  }

  controller.update();
  delay(500);
}
