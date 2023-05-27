#pragma once

#include "VoiceRecognitionV3.h"

#define VR_TAG "VR"

// #define RXD2 16
// #define TXD2 17

class VoiceRecognitionController;
typedef std::function<void(int cmd)> VoiceRecognitionCallback;

class VoiceRecognitionController {
public:
  VoiceRecognitionController();
  ~VoiceRecognitionController();
  int init(VoiceRecognitionCallback callback);
  void loop();

private:
  VR vr;
  VoiceRecognitionCallback proc;
  uint8_t buf[255];
  uint8_t records[7];
};
