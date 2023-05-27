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

private:
  HardwareSerial &cmdSerial;
  String cmdBuffer = "";
  CommandCallnack proc;
};

extern Commander Commander1;

void setupCommander();
