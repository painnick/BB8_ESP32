#pragma once

#include <Arduino.h>

#include "esp_log.h"
#include <functional>

#define COMMAND_ROUTER_TAG "CRT"

#define PIN_CMD_RX 15
#define PIN_CMD_TX 25

class CommandRouter;

typedef std::function<void(const CommandRouter *, const String &cmd)>
        CommandCallback;

class CommandRouter {
public:
    explicit CommandRouter(HardwareSerial &serial);

    ~CommandRouter();

    void init(CommandCallback callback);

    void loop();

    void send(const char *msg);

private:
    HardwareSerial &cmdSerial;
    String cmdBuffer = "";
    CommandCallback proc;
};

extern CommandRouter commandRouter1;
