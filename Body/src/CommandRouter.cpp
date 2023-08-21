#include "CommandRouter.h"

#define COMMAND_DELIMITER "/|"
#define COMMAND_DELIMITER_SIZE 2
#define MAX_COMMAND_BUFFER_SIZE 50

CommandRouter commandRouter1(Serial1);

CommandRouter::CommandRouter(HardwareSerial &serial) : cmdSerial(serial) {}
CommandRouter::~CommandRouter() {}

void CommandRouter::init(CommandCallback callback) {
  proc = callback;
  cmdSerial.begin(9600, SERIAL_8N1, PIN_CMD_RX, PIN_CMD_TX);
  delay(500);
#ifdef DEBUG
  ESP_LOGI(COMMAND_ROUTER_TAG, "Setup Command-Serial");
#endif
}

void CommandRouter::loop() {
  // unsigned long now = millis();
  // if (now - lastKeepAliveTime > 1000 * 10) {
  //   lastKeepAliveTime = now;
  //   cmdSerial.printf("Keep from BODY");
  //   cmdSerial.printf(COMMAND_DELIMITER);
  //   cmdSerial.flush();
  //   ESP_LOGD(COMMAND_ROUTER_TAG, "(BODY) => Keep from BODY");
  // }

  if (cmdSerial.available()) {
    // Append command-buffer
    while (cmdSerial.available()) {
      cmdBuffer += (char)cmdSerial.read();
    }
    // Check size of command-buffer
    if (cmdBuffer.length() > MAX_COMMAND_BUFFER_SIZE) {
      cmdBuffer = "";
    } else {
      while (-1 != cmdBuffer.indexOf(COMMAND_DELIMITER)) {
        int found = cmdBuffer.indexOf(COMMAND_DELIMITER);
        if (found != -1) {
          String cmd = cmdBuffer.substring(0, found);
          cmdBuffer = cmdBuffer.substring(found + COMMAND_DELIMITER_SIZE);
#ifdef DEBUG
          ESP_LOGV(COMMAND_ROUTER_TAG, "(HEAD) <= %s", cmd.c_str());
#endif
          proc(this, cmd);
        }
      }
    }
  }
}

void CommandRouter::send(const char *msg) {
  cmdSerial.printf(msg);
  cmdSerial.printf(COMMAND_DELIMITER);
  cmdSerial.flush();
#ifdef DEBUG
  ESP_LOGD(COMMAND_ROUTER_TAG, "(BODY) => %s", msg);
#endif
}
