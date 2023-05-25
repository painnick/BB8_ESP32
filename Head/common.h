#pragma once

#include <Arduino.h>

#include "esp_log.h"

#define USE_SERIAL
#define MAIN_TAG "Main"
#define cmdSerial Serial1

#define COMMAND_DELIMETER "\r\n"
#define COMMAND_DELIMETER_SIZE 2
#define MAX_COMMAND_BUFFER_SZIE 50

