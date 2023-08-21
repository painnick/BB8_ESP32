#include <Arduino.h>

#include "esp_log.h"

#include "BluetoothSerial.h"

#define MAIN_TAG "Main"

// These are all GPIO pins on the ESP32
// Recommended pins include 2,4,12-19,21-23,25-27,32-33
// for the ESP32-S2 the GPIO pins are 1-21,26,33-42

#define PIN_FLASH_LED 4
#define PIN_INTERNAL_LED 33

#define COMMAND_REPETITION_COUNT 5

#ifdef DEBUG
#define SAY_ANYTHING_INTERVAL_SEC 30
#else
#define SAY_ANYTHING_INTERVAL_SEC 60
#endif

#include "CommandRouter.h"
#include "MonoEyeController.h"
#include "MotorController.h"
#include "Mp3Controller.h"
#include "ShiftRegisterController.h"
#include "VoiceRecognitionController.h"
#include "LighterController.h"
#include <EEPROM.h>

#define EEPROM_SIZE 2
#define EEPROM_ADDR_VOLUME 0

BluetoothSerial SerialBT;

void randomMoveMotor(unsigned long duration, const MotorCallback &callback = nullptr,
                     unsigned long startDelayMs = 0) {
    if ((random(1024) % 2) == 0)
        motor1.left(duration, callback, startDelayMs);
    else
        motor1.right(duration, callback, startDelayMs);
}

void moveMotorOpposite(MotorController *mc, MOTOR_DIRECTION dir,
                       unsigned long duration,
                       const MotorCallback &callback = nullptr) {
    if (dir == MOTOR_DIRECTION::LEFT) {
        mc->stop();
        delay(100);
        mc->right(duration, callback);
    } else if (dir == MOTOR_DIRECTION::RIGHT) {
        mc->stop();
        delay(100);
        mc->left(duration, callback);
    }
}

void commandHello() {
    vr.loadWakeup();
    monoEyeController.wakeUp();
    randomPlayGeneral();
    randomMoveMotor(
            200 + random(0, 5) * 100,
            [](MotorController *mc, MOTOR_DIRECTION dir) -> void {
                moveMotorOpposite(mc, dir, 200 + random(0, 5) * 100);
            },
            1000);
    commandRouter1.send("LED1ON");
    commandRouter1.send("LED2ON");
    commandRouter1.send("LED3ON");
    commandRouter1.send("WIFION");
    shiftRegister.set(0xFF);
}

void commandBye() {
    vr.loadDefault();
    monoEyeController.sleep();
    playBye();
    motor1.stop();
    commandRouter1.send("LED1OFF");
    commandRouter1.send("LED2OFF");
    commandRouter1.send("LED3OFF");
    commandRouter1.send("WIFIOFF");
    shiftRegister.set(0);
}

void commandStop() {
    dfmp3.stop();
    lighterController.off();
    shiftRegister.randomLight(false);
    motor1.stop();
}

void commandRight() {
    motor1.right(500);
}

void commandLeft() {
    motor1.left(500);
}

void commandFool() {
    playFail();
    shiftRegister.warningMessage();
    randomMoveMotor(
            500,
            [](MotorController *mc, MOTOR_DIRECTION dir) -> void {
                moveMotorOpposite(
                        mc, dir, 500,
                        [](MotorController *mc, MOTOR_DIRECTION dir) -> void {
                            moveMotorOpposite(
                                    mc, dir, 300,
                                    [](MotorController *mc, MOTOR_DIRECTION dir) -> void {
                                        moveMotorOpposite(mc, dir, 300);
                                    });
                        });
            },
            500);
}

void commandMusic() {
    motor1.stop();
    shiftRegister.randomLight(true);
    playOST();
}

void commandLightOn() {
    lighterController.on();
}

void commandLightOff() {
    lighterController.off();
}

void commandIncreaseVolume() {
    dfmp3.increaseVolume();
    dfmp3.loop();

    uint8_t volume = dfmp3.getVolume();

    EEPROM.writeUChar(EEPROM_ADDR_VOLUME, volume);
    EEPROM.commit();

    SerialBT.print("Volume : ");
    SerialBT.println(volume);
}

void commandDecreaseVolume() {
    dfmp3.decreaseVolume();
    dfmp3.loop();

    uint8_t volume = dfmp3.getVolume();

    EEPROM.writeUChar(EEPROM_ADDR_VOLUME, volume);
    EEPROM.commit();

    SerialBT.print("Volume : ");
    SerialBT.println(volume);
}

void commandGetVolume() {
    uint8_t volume = dfmp3.getVolume();
    SerialBT.print("Volume : ");
    SerialBT.println(volume);
}

void commandSetVolume(uint8_t vol) {
    dfmp3.setVolume(vol);
    dfmp3.loop();

    uint8_t volume = dfmp3.getVolume();

    EEPROM.writeUChar(EEPROM_ADDR_VOLUME, volume);
    EEPROM.commit();

    SerialBT.print("Volume : ");
    SerialBT.println(volume);
}

void sendCommandList() {
    SerialBT.println("=== Type 'help' ===");
    SerialBT.println("hello");
    SerialBT.println("bye");
    SerialBT.println("right");
    SerialBT.println("left");
    SerialBT.println("stop");
    SerialBT.println("fool");
    SerialBT.println("music");
    SerialBT.println("light on");
    SerialBT.println("light off");
    SerialBT.println("led random");
    SerialBT.println("led off");
    SerialBT.println("volume +");
    SerialBT.println("volume -");
    SerialBT.println("volume {0~30}");
    SerialBT.println("volume");
}

int lastCommand = -1;
int duplicateCommandCount = 0;

void setup() {
#ifdef DEBUG
    ESP_LOGI(MAIN_TAG, "Setup...");
#endif

    EEPROM.begin(EEPROM_SIZE);
    int savedVolume = EEPROM.read(EEPROM_ADDR_VOLUME);
#ifdef DEBUG
    ESP_LOGI(MAIN_TAG, "Saved Volume : %d", savedVolume);
#endif

    SerialBT.begin("BB-8");
    SerialBT.register_callback([](esp_spp_cb_event_t evt, esp_spp_cb_param_t *param) {
        switch (evt) {
            case ESP_SPP_SRV_OPEN_EVT:
                sendCommandList();
                break;
            default:
                break;
        }
    });

    if (savedVolume > 30) {
        savedVolume = 15;
    }
    setupSound(savedVolume);

    vr.init([](int cmd) -> void {
        if (cmd < 0) {
            return;
        }

        if (lastCommand == cmd) {
            ++duplicateCommandCount;
        } else {
            duplicateCommandCount = 1;
        }
        lastCommand = cmd;

        if (duplicateCommandCount >= COMMAND_REPETITION_COUNT) {
#ifdef DEBUG
            ESP_LOGD(MAIN_TAG, "Same command %d times", duplicateCommandCount);
#endif
            playWhy();
            randomMoveMotor(
                    1500,
                    [](MotorController *mc, MOTOR_DIRECTION dir) -> void {
                        moveMotorOpposite(mc, dir, 500);
                    },
                    1000);
            shiftRegister.warningMessage();
            duplicateCommandCount = 0;
            return;
        }

        if (random(32) == 3) {
            playFail();
            randomMoveMotor(600, nullptr, 1000);
            return;
        }

        switch (vr.convertToSig(cmd)) {
            case VR_HELLO: // HELLO
                commandHello();
                break;
            case VR_BYE: // BYE
                commandBye();
                break;
            case VR_RIGHT: // RIGHT
                commandRight();
                break;
            case VR_LEFT: // LEFT
                commandLeft();
                break;
            case VR_STOP: // STOP
                commandStop();
                break;
            case VR_FOOL: // FOOL!
                commandFool();
                break;
            case VR_MUSIC: // MUSIC
                commandMusic();
                break;
            case VR_LIGHT:
                commandLightOn();
                break;
            default:
                dfmp3.stop();
                break;
        }
    });

    motor1.init();

    commandRouter1.init([](const CommandRouter *, const String &cmd) -> void {
#ifdef DEBUG
        ESP_LOGD(MAIN_TAG, "<= Recv : %s", cmd.c_str());
#endif
        if (cmd == "Left") {
            motor1.left(200);
        } else if (cmd == "Right") {
            motor1.right(200);
        }
    });

    randomPlayGeneral();

#ifdef DEBUG
    // Head의 기존 상태 값을 초기화
    commandRouter1.send("LED1OFF");
    commandRouter1.send("LED2OFF");
    commandRouter1.send("LED3OFF");
    commandRouter1.send("WIFIOFF");
#endif

    shiftRegister.warningMessage();

#ifdef DEBUG
    ESP_LOGI(MAIN_TAG, "Setup Body");
#endif
}

unsigned long lastSaid = 0;

void loop() {
    if (SerialBT.available()) {
        String btCmd = SerialBT.readString();
        ESP_LOGD(MAIN_TAG, "BT >> %s", btCmd.c_str());
        if (btCmd == "hello") {
            commandHello();
        } else if (btCmd == "bye") {
            commandBye();
        } else if (btCmd == "right") {
            commandRight();
        } else if (btCmd == "left") {
            commandLeft();
        } else if (btCmd == "stop") {
            commandStop();
        } else if (btCmd == "fool") {
            commandFool();
        } else if (btCmd == "music") {
            commandMusic();
        } else if (btCmd == "light on") {
            commandLightOn();
        } else if (btCmd == "light off") {
            commandLightOff();
        } else if (btCmd == "led random") {
            shiftRegister.randomLight(true);
        } else if (btCmd == "led off") {
            shiftRegister.set(0x00);
        } else if (btCmd == "volume +") {
            commandIncreaseVolume();
        } else if (btCmd == "volume -") {
            commandDecreaseVolume();
        } else if (btCmd.startsWith("volume ")) {
            uint8_t vol = 0;
            sscanf(btCmd.c_str(), "volume %u", &vol);
            commandSetVolume(vol);
        } else if (btCmd == "volume") {
            commandGetVolume();
        } else if (btCmd == "help") {
            sendCommandList();
        } else {
            SerialBT.print("Unknown command : ");
            SerialBT.println(btCmd);
        }
    }

    unsigned long now = millis();
    monoEyeController.loop(now);
    if (now - lastSaid > 1000 * SAY_ANYTHING_INTERVAL_SEC) {
        randomPlayGeneral();
        lastSaid = now;
    }
    motor1.loop(now);
    commandRouter1.loop();
    shiftRegister.loop(now);
    lighterController.loop(now);
    vr.loop();
    dfmp3.loop();
}
