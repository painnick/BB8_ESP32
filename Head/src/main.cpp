#include <Arduino.h>

#include "esp_log.h"

#include "soc/rtc_cntl_reg.h" // disable brownout problems
#include "soc/soc.h"          // disable brownout problems

#include <SoftwareSerial.h>

#define MAIN_TAG "Main"

// These are all GPIO pins on the ESP32
// Recommended pins include 2,4,12-19,21-23,25-27,32-33
// for the ESP32-S2 the GPIO pins are 1-21,26,33-42

// Left
#define PIN_USED1 12
#define PIN_USED2 13
#define PIN_RX 15
#define PIN_TX 14
#define PIN_USED3 2
#define PIN_USED4 4
// Right
#define PIN_USED5 16

#define COMMAND_DELIMETER "\r\n"
#define COMMAND_DELIMETER_SIZE 2
#define MAX_COMMAND_BUFFER_SZIE 50

SoftwareSerial cmdSerial(PIN_RX, PIN_TX);

void setup() {

  cmdSerial.begin(9600);
  pinMode(PIN_USED1, OUTPUT);
  pinMode(PIN_USED2, OUTPUT);
  pinMode(PIN_USED3, OUTPUT);
  pinMode(PIN_USED4, OUTPUT);

  // ESP_LOGI(MAIN_TAG, "Setup Head");
}

String cmdBuffer = "";
void loop() {
  // Append command-buffer
  if (cmdSerial.available()) {
    cmdBuffer += cmdSerial.readString();
  }
  // Check size of command-buffer
  if (cmdBuffer.length() > MAX_COMMAND_BUFFER_SZIE) {
    cmdBuffer = "";
    // ESP_LOGI(MAIN_TAG, "Command-buffer cleared.");
  } else {
    int found = cmdBuffer.indexOf(COMMAND_DELIMETER);
    if (found != -1) {
      String cmd = cmdBuffer.substring(0, found);
      cmdSerial.printf("OK\r\n");
      // ESP_LOGI(MAIN_TAG, "Cmd : %s", cmd);
      cmdBuffer = cmdBuffer.substring(found + COMMAND_DELIMETER_SIZE);
    }
  }
  delay(10);
}
