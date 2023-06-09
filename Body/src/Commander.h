#pragma once

#include <Arduino.h>

#include "esp_log.h"
#include <functional>

#define COMMANDER_TAG "CMD"

#define PIN_CMD_RX 15
#define PIN_CMD_TX 25

class Commander;
typedef std::function<void(const Commander *, const String &cmd)>
    CommandCallback;

class Commander {
public:
  Commander(HardwareSerial &serial);
  ~Commander();
  void init(CommandCallback callback);
  void loop();
  void send(const char *msg);

private:
  HardwareSerial &cmdSerial;
  String cmdBuffer = "";
  CommandCallback proc;

  unsigned long lastKeepAliveTime = 0;
};

extern Commander commander1;
