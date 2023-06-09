#include "VoiceRecognitionController.h"

#include "esp_log.h"

VoiceRecognitionController::VoiceRecognitionController() {}

VoiceRecognitionController::~VoiceRecognitionController() {}

int VoiceRecognitionController::init(VoiceRecognitionCallback callback) {
  proc = callback;

  vr.begin(9600);

  int ret = loadDefault();

#ifdef DEBUG
  ESP_LOGD(VR_TAG, "=====================================");
  ESP_LOGD(VR_TAG, "Trained");

  uint8_t signaturs[50];
  for (int i = 0; i < 8; i++) {
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

int VoiceRecognitionController::loadDefault() {
  vr.clear();

  records[0] = VR_HELLO;
  records[1] = VR_STOP;
  records[2] = VR_FOOL;
  records[3] = VR_MUSIC;

  int ret = vr.load(records, 4);
  if (ret != 0) {
#ifdef DEBUG
    ESP_LOGE(VR_TAG, "Fail to load Default record(s)(%d)", ret);
#endif
  } else {
#ifdef DEBUG
    ESP_LOGI(VR_TAG, "Default record(s) load");
#endif
  }
  return ret;
}

int VoiceRecognitionController::loadWakeup() {
  vr.clear();

  records[0] = VR_BYE;
  records[1] = VR_RIGHT;
  records[2] = VR_LEFT;
  records[3] = VR_STOP;
  records[4] = VR_FOOL;
  records[5] = VR_LIGHT;
  records[6] = VR_MUSIC;

  int ret = vr.load(records, 7);
  if (ret != 0) {
#ifdef DEBUG
    ESP_LOGE(VR_TAG, "Fail to load Default record(s)(%d)", ret);
#endif
  } else {
#ifdef DEBUG
    ESP_LOGI(VR_TAG, "Default record(s) load");
#endif
  }

  return ret;
}

int VoiceRecognitionController::convertToSig(int vrIndex) {
  return records[vrIndex];
}

VoiceRecognitionController vr;
