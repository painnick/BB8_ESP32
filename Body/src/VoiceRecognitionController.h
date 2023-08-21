#pragma once

#include "VoiceRecognitionV3.h"

#define VR_TAG "VR"

// #define RXD2 16
// #define TXD2 17

#define VR_HELLO 0
#define VR_BYE 1
#define VR_RIGHT 2
#define VR_LEFT 3
#define VR_STOP 4
#define VR_FOOL 5
#define VR_MUSIC 6
#define VR_LIGHT 7

#define MAX_COMMAND_COUNT 7

class VoiceRecognitionController;

typedef std::function<void(int cmd)> VoiceRecognitionCallback;

class VoiceRecognitionController {
public:
    VoiceRecognitionController();

    ~VoiceRecognitionController();

    int init(VoiceRecognitionCallback callback);

    void loop();

    int loadDefault();

    int loadWakeup();

    int convertToSig(int vrIndex);

private:
    VR vr;
    VoiceRecognitionCallback proc;
    uint8_t buf[255]{};
    uint8_t records[MAX_COMMAND_COUNT]{};
};

extern VoiceRecognitionController vr;