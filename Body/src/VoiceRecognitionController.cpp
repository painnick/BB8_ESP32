#include "VoiceRecognitionController.h"

#include "esp_log.h"

VoiceRecognitionController::VoiceRecognitionController() {}

VoiceRecognitionController::~VoiceRecognitionController() {}

int VoiceRecognitionController::init(VoiceRecognitionCallback callback) {
  proc = callback;

  vr.begin(9600);
  delay(500);

  // Set AUtoLoad
  records[0] = 0;
  records[1] = 1;
  records[2] = 2;
  records[3] = 3;
  int ret = vr.setAutoLoad(records, 4);
  if (ret != 0) {
    ESP_LOGE(VR_TAG, "Fail to setup VoiceRecognition(%d)", ret);
  } else {
    ESP_LOGI(VR_TAG, "Setup VoiceRecognition");
  }

  return ret;
}

void VoiceRecognitionController::loop() {
  int ret = vr.recognize(buf, 50);
  if (ret > 0) {
    ESP_LOGI(VR_TAG, "VR %d", ret);
  } else {
    int cmd = buf[2];
    proc(cmd);
  }
}
