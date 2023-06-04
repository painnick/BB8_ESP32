#include "VoiceRecognitionController.h"

#include "esp_log.h"

VoiceRecognitionController::VoiceRecognitionController() {}

VoiceRecognitionController::~VoiceRecognitionController() {}

int VoiceRecognitionController::init(VoiceRecognitionCallback callback) {
  proc = callback;

  vr.begin(9600);

  // Set AutoLoad
  for (int i = 0; i < COMMAND_COUND; i++) {
    records[i] = i;
  }
  int ret = vr.setAutoLoad(records, COMMAND_COUND + 1);
  if (ret != 0) {
#ifdef DEBUG
    ESP_LOGE(VR_TAG, "Fail to setup VoiceRecognition(%d)", ret);
#endif
  } else {
#ifdef DEBUG
    ESP_LOGI(VR_TAG, "Setup VoiceRecognition");
#endif
  }

#ifdef DEBUG
  ESP_LOGD(VR_TAG, "=====================================");
  ESP_LOGD(VR_TAG, "Trained");

  uint8_t signaturs[50];
  for (int i = 0; i < 10; i++) {
    // Clear Buffer
    for (int i = 0; i < 50; i++) {
      signaturs[i] = 0;
    }
    // Read Signature
    auto ret = vr.checkSignature(i, signaturs);
    if (ret > -1) {
      ESP_LOGI(VR_TAG, " %d. %s", i, signaturs);
    }
  }
  ESP_LOGD(VR_TAG, "=====================================");
#endif

  return ret;
}

void VoiceRecognitionController::loop() {
  int length = vr.recognize(buf, 50);
  if (length > 0) {
    char newBuf[50]{};
    int cmd = buf[2];
    int len = buf[3];
    for (int i = 0; i < len; i++) {
      newBuf[i] = buf[4 + i];
    }
#ifdef DEBUG
    ESP_LOGD(VR_TAG, "SIG %d %s", cmd, newBuf);
#endif
    proc(cmd);
  }
}

VoiceRecognitionController vr;
