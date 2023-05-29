#include "common.h"

#include "http_server.h"

#define USE_SERIAL_DEBUG

// These are all GPIO pins on the ESP32
// Recommended pins include 2,4,12-19,21-23,25-27,32-33
// for the ESP32-S2 the GPIO pins are 1-21,26,33-42

// Left
#define PIN_LED1 12
#define PIN_LED2 13
#define PIN_RX 15
#define PIN_TX 14
#define PIN_ONBOARDLED 2
#define PIN_FLASHLIGHT 4
// Right
#define PIN_LED3 16

void process(const String &cmd);

void setup() {
#ifdef USE_SERIAL_DEBUG
  Serial.begin(115200);
  Serial.setDebugOutput(true);

  delay(2000);

  Serial.println("Setup...");
#endif

  initCamera();
  initCameraServer();
  startCameraServer();
#ifdef USE_SERIAL_DEBUG
  Serial.println("Setup Camera.");
#endif

  cmdSerial.begin(9600, SERIAL_8N1, PIN_RX, PIN_TX);
#ifdef USE_SERIAL_DEBUG
  Serial.println("Setup Serial.");
#endif

  pinMode(PIN_LED1, OUTPUT);
  pinMode(PIN_LED2, OUTPUT);
  pinMode(PIN_LED3, OUTPUT);
}

unsigned long lastTime = 0;
unsigned long bufferSetUntil = 0;
String cmdBuffer = "";
void loop() {
  unsigned long now = millis();
  //   if (now - lastTime > 1000 * 10) {
  //     lastTime = now;
  //     cmdSerial.printf("Keep from HEAD");
  //     cmdSerial.printf(COMMAND_DELIMETER);
  //     cmdSerial.flush();
  // #ifdef USE_SERIAL_DEBUG
  //     Serial.println("=> Keep from HEAD");
  // #endif
  //   }

  if (cmdSerial.available()) {
    if (now > bufferSetUntil) {
      cmdBuffer = "";
#ifdef USE_SERIAL_DEBUG
      Serial.println("Buffer Timeout");
#endif
    }

    // Append command-buffer
    while (cmdSerial.available()) {
      cmdBuffer += (char)cmdSerial.read();
#ifdef USE_SERIAL_DEBUG
      Serial.println("Buffer ::" + cmdBuffer + "::");
#endif
    }
    // Check size of command-buffer
    if (cmdBuffer.length() > MAX_COMMAND_BUFFER_SZIE) {
      cmdBuffer = "";
#ifdef USE_SERIAL_DEBUG
      Serial.println("Clear Buffer!");
#endif
    } else {
      while (-1 != cmdBuffer.indexOf(COMMAND_DELIMETER)) {
        int found = cmdBuffer.indexOf(COMMAND_DELIMETER);
        if (found != -1) {
          String cmd = cmdBuffer.substring(0, found);
          cmdBuffer = cmdBuffer.substring(found + COMMAND_DELIMETER_SIZE);
#ifdef USE_SERIAL_DEBUG
          Serial.println("<= " + cmd + "===");
#endif
          process(cmd);
        }
      }
    }

    bufferSetUntil = now + 1000;
  }
}

void ackCommand(const String &cmd) {
  cmdSerial.printf("ACK.");
  cmdSerial.printf(COMMAND_DELIMETER);
  cmdSerial.flush();

#ifdef USE_SERIAL_DEBUG
  Serial.println("=> ACK " + cmd);
#endif
}

void process(const String &cmd) {
  if (cmd == "LED1ON") {
    digitalWrite(PIN_LED1, HIGH);
    ackCommand(cmd);
  } else if (cmd == "LED1OFF") {
    digitalWrite(PIN_LED1, LOW);
    ackCommand(cmd);
  } else if (cmd == "LED2ON") {
    digitalWrite(PIN_LED2, HIGH);
    ackCommand(cmd);
  } else if (cmd == "LED2OFF") {
    digitalWrite(PIN_LED2, LOW);
    ackCommand(cmd);
  } else if (cmd == "LED3ON") {
    digitalWrite(PIN_LED3, HIGH);
    ackCommand(cmd);
  } else if (cmd == "LED3OFF") {
    digitalWrite(PIN_LED3, LOW);
    ackCommand(cmd);
  } else {
    ackCommand("Unknown");
#ifdef USE_SERIAL_DEBUG
    Serial.println("Unhandled command : " + cmd);
#endif
  }
}
