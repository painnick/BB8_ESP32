#pragma once

#include <Arduino.h>

#include "esp_log.h"
#include <functional>

#define COMMANDER_TAG "CMD"

#define PIN_CMD_RX 15
#define PIN_CMD_TX 14

class Commander;
typedef std::function<void(const Commander *, const String &cmd)>
    CommandCallnack;

class Commander {
public:
  Commander(HardwareSerial &serial);
  ~Commander();
  void init(CommandCallnack callback);
  void loop();
  void send(char *msg);

private:
  HardwareSerial &cmdSerial;
  String cmdBuffer = "";
  CommandCallnack proc;
};

extern Commander commander1;
