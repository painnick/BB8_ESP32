#pragma once

#include "DFMiniMp3.h"
#include "SoftwareSerial.h"
#include "esp_log.h"

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

SoftwareSerial dfSerial(PIN_MP3_RX, PIN_MP3_TX);
DfMp3 dfmp3(dfSerial);

void setupSound() {

  dfmp3.begin(9600, 1000);
  dfmp3.reset();

  dfmp3.stop();
  dfmp3.setVolume(18);

  ESP_LOGI(MP3_TAG, "Setup DFPlayer");
}
