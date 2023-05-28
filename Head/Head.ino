#include "common.h"

#include "http_server.h"

// These are all GPIO pins on the ESP32
// Recommended pins include 2,4,12-19,21-23,25-27,32-33
// for the ESP32-S2 the GPIO pins are 1-21,26,33-42

// Left
#define PIN_LED1 12
#define PIN_LED2 13
#define PIN_RX 15
#define PIN_TX 14
#define PIN_USED3 4
// Right
#define PIN_USED4 16

void process(String& cmd);

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

  cmdSerial.begin(115200, SERIAL_8N1, PIN_RX, PIN_TX);
#ifdef USE_SERIAL_DEBUG
  Serial.println("Setup Serial.");
#endif

  pinMode(PIN_LED1, OUTPUT);
  pinMode(PIN_LED2, OUTPUT);
}

unsigned long lastTime = 0;
String cmdBuffer = "";
void loop() {
  unsigned long now = millis();
  if (now - lastTime > 1000 * 30) {
    lastTime = now;
    cmdSerial.printf("Keep.");
    cmdSerial.printf(COMMAND_DELIMETER);
    cmdSerial.flush();
#ifdef USE_SERIAL_DEBUG
      Serial.println("=> Keep.");
#endif
  }

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
#ifdef USE_SERIAL_DEBUG
      Serial.println("<= " + cmd);
#endif
      process(cmd);
    }
  }
  delay(10);
}

void process(String& cmd) {
  if (cmd == "LED1ON") {
    digitalWrite(PIN_LED1, HIGH);

    cmdSerial.printf("ACK %s", cmd.c_str());
    cmdSerial.printf(COMMAND_DELIMETER);
    cmdSerial.flush();
  }
  else if (cmd == "LED1OFF") {
    digitalWrite(PIN_LED1, LOW);

    cmdSerial.printf("ACK %s", cmd.c_str());
    cmdSerial.printf(COMMAND_DELIMETER);
    cmdSerial.flush();
  }
  else if (cmd == "LED2ON") {
    digitalWrite(PIN_LED2, HIGH);

    cmdSerial.printf("ACK %s", cmd.c_str());
    cmdSerial.printf(COMMAND_DELIMETER);
    cmdSerial.flush();
  }
  else if (cmd == "LED2OFF") {
    digitalWrite(PIN_LED2, LOW);

    cmdSerial.printf("ACK %s", cmd.c_str());
    cmdSerial.printf(COMMAND_DELIMETER);
    cmdSerial.flush();
  }
  else {
#ifdef USE_SERIAL_DEBUG
    Serial.println("Unhandled command : " + cmd);
#endif
  }
}
