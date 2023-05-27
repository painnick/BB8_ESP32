#include "Commander.h"

#define COMMAND_DELIMETER "\r\n"
#define COMMAND_DELIMETER_SIZE 2
#define MAX_COMMAND_BUFFER_SZIE 50

Commander commander1(Serial1);

Commander::Commander(HardwareSerial &serial) : cmdSerial(serial) {}
Commander::~Commander() {}

void Commander::init(CommandCallnack callback) {
  proc = callback;
  cmdSerial.begin(115200, SERIAL_8N1, PIN_CMD_RX, PIN_CMD_TX);
  delay(500);
  ESP_LOGI(COMMANDER_TAG, "Setup Command-Serial");
}

void Commander::loop() {
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
        ESP_LOGI(COMMANDER_TAG, "<= %s", cmd.c_str());
        proc(this, cmd);
      }
    }
  }
}

void Commander::send(char *msg) {
  cmdSerial.printf("%s\r\n", msg);
  cmdSerial.flush();
  ESP_LOGI(COMMANDER_TAG, "=> %s", msg);
}
