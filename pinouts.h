#ifndef PINOUTS_H
#define PINOUTS_H

#include <Arduino.h>
#include "types.h"

// Define all available hardware configurations

// Available configurations as enum for easier selection
enum ConfigType {
    NANOFRAME,
    BREACH_KEY,
    AURORA_GLASYA,
    BLINDER_MINI,
    AG_ECHO_FRAME
};

struct PinConfig {
    ledSegment leds[2];
    uint8_t colorButton;
    uint8_t animButton;
};

// Configuration for Aurora Glasya
const PinConfig NANOFRAME_PINOUT = {
    .leds = {
        {PC4, PC5, PC6}, // Upper LEDs
        {PD4, PD5, PD3}  // Lower LEDs
    },
    .colorButton = PD6,
    .animButton = PD7
};

// Configuration for Breach Key RSK5
const PinConfig BREACH_KEY_PINOUT = {
    .leds = {
        {PC4, PC5, PC6}, // Upper LEDs
        {PD4, PD5, PD3}  // Lower LEDs
    },
    .colorButton = PD2,
    .animButton = PC7
};

// Configuration for Aurora Glasya
const PinConfig AURORA_GLASYA_PINOUT = {
    .leds = {
        {PC4, PC5, PC6}, // Upper LEDs
        {PD4, PD5, PD3}  // Lower LEDs
    },
    .colorButton = PD2,
    .animButton = PC7
};

// Configuration for Blinder Mini
const PinConfig BLINDER_MINI_PINOUT = {
    .leds = {
        {PC4, PC5, PC6}, // Upper LEDs
        {PD4, PD5, PD3}  // Lower LEDs
    },
    .colorButton = PD2,
    .animButton = PC7
};

// Configuration for AG Echo Frame
const PinConfig AG_ECHO_FRAME_PINOUT = {
    .leds = {
        {PD7, PD6, PD5}, // Eyepiece
        {PD2, PD3, PD4}  // Main body
    },
    .colorButton = PC3,
    .animButton = PC0
};

// Function to get the active configuration based on the selected hardware
inline const PinConfig& getActiveConfig(ConfigType configType) {
    switch(configType) {
        case NANOFRAME:
            return NANOFRAME_PINOUT;
        case BREACH_KEY:
            return BREACH_KEY_PINOUT;
        case AURORA_GLASYA:
            return AURORA_GLASYA_PINOUT;
        case BLINDER_MINI:
            return BLINDER_MINI_PINOUT;
        case AG_ECHO_FRAME:
        default:
            return AG_ECHO_FRAME_PINOUT;
    }
}

#endif // PINOUTS_H
