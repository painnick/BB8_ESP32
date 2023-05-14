#include <Arduino.h>

#include "esp_log.h"

#include "soc/soc.h"             // disable brownout problems
#include "soc/rtc_cntl_reg.h"    // disable brownout problems

#define MAIN_TAG "Main"

// These are all GPIO pins on the ESP32
// Recommended pins include 2,4,12-19,21-23,25-27,32-33
// for the ESP32-S2 the GPIO pins are 1-21,26,33-42

// for ESP32 Microcontrollers
#define PIN_DATA  21
#define PIN_LATCH 22
#define PIN_CLOCK 23
 
byte shift_register = 0;

void updateShiftRegister() {
   digitalWrite(PIN_LATCH, LOW);
   shiftOut(PIN_DATA, PIN_CLOCK, LSBFIRST, shift_register);
   digitalWrite(PIN_LATCH, HIGH);
}

void setup() {
  pinMode(PIN_DATA, OUTPUT);  
  pinMode(PIN_LATCH, OUTPUT);
  pinMode(PIN_CLOCK, OUTPUT);

  bitSet(shift_register, 0);
  updateShiftRegister();
}
 
void loop() {
}
