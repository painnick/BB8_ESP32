#pragma once

#include "VoiceRecognitionV3.h"

#define VR_TAG "VR"

// #define RXD2 16
// #define TXD2 17

VR myVR;
uint8_t buf[255];

void setupVR() {
  myVR.begin(9600);
  delay(500);
  uint8_t records[7];
  records[0] = 0;
  records[1] = 1;
  records[2] = 2;
  records[3] = 3;
  int ret = myVR.setAutoLoad(records, 4);
  if (ret != 0) {
    ESP_LOGE(VR_TAG, "Fail to setup VoiceRecognition(%d)", ret);
  } else {
    ESP_LOGI(VR_TAG, "Setup VoiceRecognition");
  }
}
