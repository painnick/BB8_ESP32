#include "Commander.h"

#define COMMAND_DELIMETER "/|"
#define COMMAND_DELIMETER_SIZE 2
#define MAX_COMMAND_BUFFER_SZIE 50

Commander commander1(Serial1);

Commander::Commander(HardwareSerial &serial) : cmdSerial(serial) {}
Commander::~Commander() {}

void Commander::init(CommandCallback callback) {
  proc = callback;
  cmdSerial.begin(9600, SERIAL_8N1, PIN_CMD_RX, PIN_CMD_TX);
  delay(500);
  ESP_LOGI(COMMANDER_TAG, "Setup Command-Serial");
}

void Commander::loop() {
  // unsigned long now = millis();
  // if (now - lastKeepAliveTime > 1000 * 10) {
  //   lastKeepAliveTime = now;
  //   cmdSerial.printf("Keep from BODY");
  //   cmdSerial.printf(COMMAND_DELIMETER);
  //   cmdSerial.flush();
  //   ESP_LOGD(COMMANDER_TAG, "(BODY) => Keep from BODY");
  // }

  if (cmdSerial.available()) {
    // Append command-buffer
    while (cmdSerial.available()) {
      cmdBuffer += (char)cmdSerial.read();
    }
    // Check size of command-buffer
    if (cmdBuffer.length() > MAX_COMMAND_BUFFER_SZIE) {
      cmdBuffer = "";
    } else {
      while (-1 != cmdBuffer.indexOf(COMMAND_DELIMETER)) {
        int found = cmdBuffer.indexOf(COMMAND_DELIMETER);
        if (found != -1) {
          String cmd = cmdBuffer.substring(0, found);
          cmdBuffer = cmdBuffer.substring(found + COMMAND_DELIMETER_SIZE);
          ESP_LOGV(COMMANDER_TAG, "(HEAD) <= %s", cmd.c_str());
          proc(this, cmd);
        }
      }
    }
  }
}

void Commander::send(const char *msg) {
  cmdSerial.printf(msg);
  cmdSerial.printf(COMMAND_DELIMETER);
  cmdSerial.flush();
  ESP_LOGD(COMMANDER_TAG, "(BODY) => %s", msg);
}
