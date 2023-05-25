#include <Arduino.h>

#include "esp_log.h"

#include "soc/rtc_cntl_reg.h" // disable brownout problems
#include "soc/soc.h"          // disable brownout problems

#define MAIN_TAG "Main"

// These are all GPIO pins on the ESP32
// Recommended pins include 2,4,12-19,21-23,25-27,32-33
// for the ESP32-S2 the GPIO pins are 1-21,26,33-42

#define PIN_MP3_RX 13
#define PIN_MP3_TX 27

#define PIN_DATA 23
#define PIN_LATCH 22
#define PIN_CLOCK 21

#define PIN_RX 15
#define PIN_TX 14

#define PIN_FLASH_LED 4
#define PIN_INTERNAL_LED 33

#define PIN_MOTOR1 18
#define PIN_MOTOR2 19

#define CHANNEL_MOTOR1 14
#define CHANNEL_MOTOR2 15

// #define USE_SOUND
// #define USE_COMMANDER
// #define USE_VR
// #define USE_SHIFT_REGISTER

#ifdef USE_COMMANDER
#include "SoftwareSerial.h"
#endif

#ifdef USE_VR
#include "VoiceRecognitionV3.h"
#endif

#ifdef USE_SHIFT_REGISTER
#include "ShiftRegisterController.h"

ShiftRegisterController shiftRegister(PIN_DATA, PIN_LATCH, PIN_CLOCK);
#endif

#ifdef USE_VR
VR myVR;
uint8_t buf[255];
uint8_t records[7];
#endif

#ifdef USE_SOUND
#define MP3_TAG "DFPlayer"

#include "DFMiniMp3.h"

class Mp3Notify;
typedef DFMiniMp3<SoftwareSerial, Mp3Notify> DfMp3;
SoftwareSerial dfplayer(PIN_MP3_RX, PIN_MP3_TX);
DfMp3 dfmp3(dfplayer);
#endif

#ifdef USE_COMMANDER
#define cmdSerial Serial1

#define COMMAND_DELIMETER "\r\n"
#define COMMAND_DELIMETER_SIZE 2
#define MAX_COMMAND_BUFFER_SZIE 50

String cmdBuffer = "";
#endif

#ifdef USE_VR
void setupVR() {
  myVR.begin(9600);
  delay(500);
  records[0] = 0;
  records[1] = 1;
  records[2] = 2;
  records[3] = 3;
  int ret = myVR.setAutoLoad(records, 4);
  if (ret != 0) {
    ESP_LOGE(MAIN_TAG, "Fail to setup VoiceRecognition(%d)", ret);
  } else {
    ESP_LOGI(MAIN_TAG, "Setup VoiceRecognition");
  }
}
#endif

#ifdef USE_SOUND
void setupSound() {
  dfmp3.begin(9600, 1000);
  dfmp3.reset();

  while (!dfmp3.isOnline()) {
    delay(500);
    ESP_LOGI(MAIN_TAG, "(dfmp3)...");
  }

  dfmp3.setVolume(18);

  ESP_LOGI(MAIN_TAG, "Setup DFPlayer");
}
#endif

#ifdef USE_COMMANDER
void setupCommander() {
  cmdSerial.begin(115200, SERIAL_8N1, PIN_RX, PIN_TX);
  delay(500);
  ESP_LOGI(MAIN_TAG, "Setup Command-Serial");
}
#endif

#ifdef USE_SHIFT_REGISTER
void setupShiftRegister() {
  pinMode(PIN_INTERNAL_LED, OUTPUT);
  shiftRegister.set(0);
  shiftRegister.update();
}
#endif

void setupMotor() {
  ledcSetup(CHANNEL_MOTOR1, 1000, 8); // 0~255
  ledcSetup(CHANNEL_MOTOR2, 1000, 8); // 0~255

  ledcAttachPin(PIN_MOTOR1, CHANNEL_MOTOR1);
  ledcAttachPin(PIN_MOTOR2, CHANNEL_MOTOR2);

  ledcWrite(CHANNEL_MOTOR1, 0);
  ledcWrite(CHANNEL_MOTOR2, 255);
}

void setup() {

  delay(1000);

#ifdef USE_VR
  setupVR();
#endif

#ifdef USE_SOUND
  setupSound();
#endif

#ifdef USE_COMMANDER
  setupCommander();
#endif

#ifdef USE_SHIFT_REGISTER
  setupShiftRegister();
#endif

  setupMotor();

  ESP_LOGI(MAIN_TAG, "Setup Body");
}

int song = 0;
void loop() {

#ifdef USE_COMMANDER
  // Check Command
  if (cmdSerial.available()) {
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
        ESP_LOGI(MAIN_TAG, "<= %s", cmd.c_str());

        if (cmd == "PLAY1") {
          int newSong = 1;
          if (song != newSong) {
#ifdef USE_SOUND
            dfmp3.nextTrack();
#endif
            song = newSong;
          }
        } else if (cmd == "PLAY2") {
          int newSong = 0;
          if (song != newSong) {
#ifdef USE_SOUND
            dfmp3.stop();
#endif
            song = newSong;
          }
        }

        byte curVal = 0;
        shiftRegister.set(bitSet(curVal, 6));
        // delay(10);
      }
    }
  } else {
    shiftRegister.set(0);
  }
#endif

#ifdef USE_VR
  // Check VoiceRecognition
  int ret = myVR.recognize(buf, 50);
  if (ret > 0) {
#ifdef USE_COMMANDER
    int cmd = buf[2];
    cmdSerial.printf("LED%d\r\n", cmd);
    cmdSerial.flush();
    ESP_LOGI(MAIN_TAG, "=> LED%d", cmd);

    ESP_LOGI(MAIN_TAG, "!!!Command!!! %d", cmd);
#endif
  }
#endif

#ifdef USE_SHIFT_REGISTER
  shiftRegister.update();
#endif

#ifdef USE_SOUND
  dfmp3.loop();
#endif
}

#ifdef USE_SOUND
//----------------------------------------------------------------------------------
class Mp3Notify {
public:
  static void PrintlnSourceAction(DfMp3_PlaySources source,
                                  const char *action) {
    if (source & DfMp3_PlaySources_Sd) {
      ESP_LOGD(MP3_TAG, "SD Card, %s", action);
    }
    if (source & DfMp3_PlaySources_Usb) {
      ESP_LOGD(MP3_TAG, "USB Disk, %s", action);
    }
    if (source & DfMp3_PlaySources_Flash) {
      ESP_LOGD(MP3_TAG, "Flash, %s", action);
    }
  }
  static void OnError(DfMp3 &mp3, uint16_t errorCode) {
    // see DfMp3_Error for code meaning
    Serial.println();
    Serial.print("Com Error ");
    switch (errorCode) {
    case DfMp3_Error_Busy:
      ESP_LOGE(MP3_TAG, "Com Error - Busy");
      break;
    case DfMp3_Error_Sleeping:
      ESP_LOGE(MP3_TAG, "Com Error - Sleeping");
      break;
    case DfMp3_Error_SerialWrongStack:
      ESP_LOGE(MP3_TAG, "Com Error - Serial Wrong Stack");
      break;

    case DfMp3_Error_RxTimeout:
      ESP_LOGE(MP3_TAG, "Com Error - Rx Timeout!!!");
      break;
    case DfMp3_Error_PacketSize:
      ESP_LOGE(MP3_TAG, "Com Error - Wrong Packet Size!!!");
      break;
    case DfMp3_Error_PacketHeader:
      ESP_LOGE(MP3_TAG, "Com Error - Wrong Packet Header!!!");
      break;
    case DfMp3_Error_PacketChecksum:
      ESP_LOGE(MP3_TAG, "Com Error - Wrong Packet Checksum!!!");
      break;

    default:
      ESP_LOGE(MP3_TAG, "Com Error - %d", errorCode);
      break;
    }
  }
  static void OnPlayFinished(DfMp3 &mp3, DfMp3_PlaySources source,
                             uint16_t track) {
    ESP_LOGD(MP3_TAG, "Play finished for #%d", track);
  }
  static void OnPlaySourceOnline(DfMp3 &mp3, DfMp3_PlaySources source) {
    PrintlnSourceAction(source, "online");
  }
  static void OnPlaySourceInserted(DfMp3 &mp3, DfMp3_PlaySources source) {
    PrintlnSourceAction(source, "inserted");
  }
  static void OnPlaySourceRemoved(DfMp3 &mp3, DfMp3_PlaySources source) {
    PrintlnSourceAction(source, "removed");
  }
};
#endif
