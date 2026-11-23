#ifndef PINOUTS_H
#define PINOUTS_H

#include <Arduino.h>
#include "types.h"

// Define all available hardware configurations

// Available configurations as enum for easier selection
enum ConfigType {
    CONFIG_BLINDER_MINI,
    CONFIG_AG_ECHO_FRAME
};

struct PinConfig {
    ledSegment leds[2];
    uint8_t colorButton;
    uint8_t animButton;
};

// Configuration for Blinder Mini
const PinConfig BLINDER_MINI = {
    .leds = {
        {PC4, PC5, PC6}, // Upper LEDs
        {PD3, PD4, PD5}  // Lower LEDs
    },
    .colorButton = PD6,
    .animButton = PD2
};

// Configuration for AG Echo Frame
const PinConfig AG_ECHO_FRAME = {
    .leds = {
        {PD7, PD6, PD5}, // Eyepiece
        {PD2, PD3, PD4}  // Main body
    },
    .colorButton = PC3,
    .animButton = PC0
};

// Function to get the active configuration based on the selected hardware
// Pass in CONFIG_BLINDER_MINI or CONFIG_AG_ECHO_FRAME
inline const PinConfig& getActiveConfig(ConfigType configType) {
    switch(configType) {
        case CONFIG_BLINDER_MINI:
            return BLINDER_MINI;
        case CONFIG_AG_ECHO_FRAME:
        default:
            return AG_ECHO_FRAME;
    }
}

#endif // PINOUTS_H
