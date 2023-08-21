#pragma once

#include <Arduino.h>

#define PIN_LIGHTER 14

class LighterController {
public:
    explicit LighterController(uint8_t pinNo);

    void on();

    void off();

    void loop(unsigned long now);

private:
    const uint8_t ledChannel{7};
    boolean isOn{false};
    boolean isChanged{false};
    unsigned int lastChecked{};
};

extern LighterController lighterController;
