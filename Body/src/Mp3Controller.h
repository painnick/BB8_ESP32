#pragma once

#include "DFMiniMp3.h"
#include "SoftwareSerial.h"
#include "esp_log.h"

#include "ShiftRegisterController.h"

#define MP3_TAG "DFPLAYER"

#define PIN_MP3_RX 26
#define PIN_MP3_TX 27

class Mp3Notify;
typedef DFMiniMp3<SoftwareSerial, Mp3Notify> DfMp3;

class Mp3Notify {
public:
  static void PrintlnSourceAction(DfMp3_PlaySources source,
                                  const char *action) {
    if (source & DfMp3_PlaySources_Sd) {
#ifdef DEBUG
      ESP_LOGD(MP3_TAG, "SD Card, %s", action);
#endif
    }
    if (source & DfMp3_PlaySources_Usb) {
#ifdef DEBUG
      ESP_LOGD(MP3_TAG, "USB Disk, %s", action);
#endif
    }
    if (source & DfMp3_PlaySources_Flash) {
#ifdef DEBUG
      ESP_LOGD(MP3_TAG, "Flash, %s", action);
#endif
    }
  }
  static void OnError(DfMp3 &mp3, uint16_t errorCode) {
    // see DfMp3_Error for code meaning
    switch (errorCode) {
    case DfMp3_Error_Busy:
#ifdef DEBUG
      ESP_LOGE(MP3_TAG, "Com Error - Busy");
#endif
      break;
    case DfMp3_Error_Sleeping:
#ifdef DEBUG
      ESP_LOGE(MP3_TAG, "Com Error - Sleeping");
#endif
      break;
    case DfMp3_Error_SerialWrongStack:
#ifdef DEBUG
      ESP_LOGE(MP3_TAG, "Com Error - Serial Wrong Stack");
#endif
      break;

    case DfMp3_Error_RxTimeout:
#ifdef DEBUG
      ESP_LOGE(MP3_TAG, "Com Error - Rx Timeout!!!");
#endif
      break;
    case DfMp3_Error_PacketSize:
#ifdef DEBUG
      ESP_LOGE(MP3_TAG, "Com Error - Wrong Packet Size!!!");
#endif
      break;
    case DfMp3_Error_PacketHeader:
#ifdef DEBUG
      ESP_LOGE(MP3_TAG, "Com Error - Wrong Packet Header!!!");
#endif
      break;
    case DfMp3_Error_PacketChecksum:
#ifdef DEBUG
      ESP_LOGE(MP3_TAG, "Com Error - Wrong Packet Checksum!!!");
#endif
      break;

    default:
#ifdef DEBUG
      ESP_LOGE(MP3_TAG, "Com Error - %d", errorCode);
#endif
      break;
    }
  }
  static void OnPlayFinished(DfMp3 &mp3, DfMp3_PlaySources source,
                             uint16_t track) {
#ifdef DEBUG
    ESP_LOGD(MP3_TAG, "Play finished for #%d", track);
#endif
    shiftRegister.randomLight(false);
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

SoftwareSerial dfSerial(PIN_MP3_RX, PIN_MP3_TX);
DfMp3 dfmp3(dfSerial);

void setupSound() {

  dfmp3.begin(9600, 1000);
  delay(1000);
  while (!dfSerial.available()) {
    delay(1);
  }

#ifdef DEBUG
  dfmp3.setVolume(15);
  dfmp3.loop();
  ESP_LOGI(MP3_TAG, "Set Volume 15");

  ESP_LOGI(MP3_TAG, "Setup DFPlayer");
#endif
}

void randomPlayGeneral() {
  // 02/xxxx .mp3
  int index = random(1, 5);
  dfmp3.playFolderTrack16(2, index);
}

void playOST() {
  // 01/xxxx .mp3
  int index = random(1, 4);
  dfmp3.playFolderTrack16(1, index);
}

void playFail() {
  // mp3/0003 .mp3
  dfmp3.playMp3FolderTrack(3);
}

void playWhy() {
  // mp3/0004 .mp3
  dfmp3.playMp3FolderTrack(4);
}

void playBye() {
  // mp3/0005 .mp3
  dfmp3.playMp3FolderTrack(5);
}
