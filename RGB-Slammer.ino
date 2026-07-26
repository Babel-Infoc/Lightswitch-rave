/*
RGB Slammer
Written by Tully Jagoe 2025
MIT License

This script is best edited in VSCode for color token selection in swatches.cpp
*/

#include <Arduino.h>
#include "types.h"
#include "swatches.h"
#include "waveforms.h"
#include "flashStorage.h"
#include "pinouts.h"

// Forward declarations
void animationPreview();

// Number of LED segments
const uint8_t numLEDs = 2;

// Select which hardware configuration to use
ConfigType activeConfig = AG_ECHO_FRAME;

// Define the LED array and button pins according to the active configuration
ledSegment led[2];
uint8_t colorBtn;
uint8_t animBtn;

// Set the default brightness modifier, 0.0 to 0.65 max
float currentBrightness = 0.5;

// Brightness adjustment settings
const float minBrightness = 0.2;
const float maxBrightness = 0.6;
const unsigned long brightnessModeTriggerTime = 500; // milliseconds to hold button to enter brightness mode
bool brightnessAdjustMode = false;

// Slow down all animations by this amou nt (in milliseconds)
const uint8_t slowDown = 1;

// LED Color tuning
// Define the light intensity of each LED color at the specified mA value
// Check your LEDs datasheet for typical luminosity values for standard forward current
// {mA, luminosity}
const luminance red       = {5, 45};
const luminance green     = {5, 45};
const luminance blue      = {5, 55};

// -------------------------------------------------------------------------------------
// MARK: Setup
void setup() {
    // Get the active configuration using the case-based approach
    const PinConfig& config = getActiveConfig(activeConfig);

    // Copy pin configuration from the selected hardware profile
    led[0] = config.leds[0];
    led[1] = config.leds[1];
    colorBtn = config.colorButton;
    animBtn = config.animButton;

    // Set up all LED segments
    for (uint8_t segment = 0; segment < 2; segment++) {
        pinMode(led[segment].red, OUTPUT);
        pinMode(led[segment].green, OUTPUT);
        pinMode(led[segment].blue, OUTPUT);
    }
    // Set up the buttons
    pinMode(colorBtn, INPUT_PULLUP);
    pinMode(animBtn, INPUT_PULLUP);

    // Calculate the luminosity modifiers
    calculateLuminance();

    // Set up random seeds
    float randSeed1(analogRead(0));
    float randSeed2(analogRead(1));


    // Try to load saved settings from flash
    if (!loadSettingsFromFlash(&swNum, &currentBrightness, &animationMode)) {
        // If no valid settings found, use defaults (which are already set in declarations)
        swNum = 23;
        currentBrightness = 0.4; // Default brightness
        animationMode = 0; // Default to glitchLoop
    }

    // Show the bootup animation
    bounceBoot(40);
}

/*=======================================================================================
// MARK:                                Main loop                                      //
=======================================================================================*/
// Only runs the glitchLoop animation

void loop() {
    // Check if brightness adjustment mode should be active
    if (brightnessAdjustMode) {
        brightnessAdjustmentMode();
    }
    // Check if swatch preview animation should play
    else if (swatchPreviewActive) {
        swatchPreview();
    }
    // Check if animation preview should play
    else if (animationPreviewActive) {
        animationPreview();
    } else {
        // Run the selected animation mode
        switch (animationMode) {
            case 0:
            default:
                glitchLoop(70, 20, 1000);
                break;
            case 1:
                slowFade();
                break;
            case 2:
                photomode1();
                break;
            case 3:
                photomode2();
                break;
        }
    }
}

/*=======================================================================================
//                                    End main loop                                    //
=======================================================================================*/

// -------------------------------------------------------------------------------------
// MARK: bounceBoot
void bounceBoot(int speed){
    for (uint8_t reps = 0; reps < 3; reps++) {
        if (reps == 2) speed = speed * 3;
        fadeToColor(swatch[swNum].primary,      swatch[swNum].background,   speed);
        fadeToColor(swatch[swNum].accent,       swatch[swNum].primary,      speed);
        fadeToColor(swatch[swNum].midtone,      swatch[swNum].accent,       speed);
        fadeToColor(swatch[swNum].contrast,     swatch[swNum].midtone,      speed);
        fadeToColor(swatch[swNum].background,   swatch[swNum].contrast,     speed);
        fadeToColor(swatch[swNum].background,   swatch[swNum].background,   speed);
    }
    showColor(swatch[0].background, swatch[0].background, speed*5);
}

// MARK: photomode1
void photomode1() {
    while (true) {
        if (buttonInterruptCheck()) return;
        // showColor handles all segments internally: color1→ROLE_GPIO, color2→ROLE_SR
        showColor(swatch[swNum].primary, swatch[swNum].primary, 10);
    }
}
// MARK: photomode2
void photomode2() {
    while (true) {
        if (buttonInterruptCheck()) return;
        // showColor handles all segments internally: color1→ROLE_GPIO, color2→ROLE_SR
        showColor(swatch[swNum].primary, swatch[swNum].contrast, 10);
    }
}

// MARK: photomode2
void photomode3() {
    while (true) {
        if (buttonInterruptCheck()) return;
        // showColor handles all segments internally: color1→ROLE_GPIO, color2→ROLE_SR
        showColor(swatch[swNum].primary, swatch[swNum].background, 10);
    }
}

// MARK: slowFade
// Breathing effect: fades through swatch colors (background→primary) over 1s, then reverses over 3s.
void slowFade() {
    const uint16_t FADE_UP_TIME   = 250;  // 1000ms / 4 transitions = 250ms each
    const uint16_t FADE_DOWN_TIME = 750;  // 3000ms / 4 transitions = 750ms each

    while (true) {
        if (buttonInterruptCheck()) return;

        // Fade up: background → contrast → midtone → accent → primary (1 second total)
        fadeToColor(swatch[swNum].contrast, swatch[swNum].contrast, FADE_UP_TIME);
        if (buttonInterruptCheck()) return;

        fadeToColor(swatch[swNum].midtone, swatch[swNum].midtone, FADE_UP_TIME);
        if (buttonInterruptCheck()) return;

        fadeToColor(swatch[swNum].accent, swatch[swNum].accent, FADE_UP_TIME);
        if (buttonInterruptCheck()) return;

        fadeToColor(swatch[swNum].primary, swatch[swNum].primary, FADE_UP_TIME);
        if (buttonInterruptCheck()) return;

        // Fade down: primary → accent → midtone → contrast → background (3 seconds total)
        fadeToColor(swatch[swNum].accent, swatch[swNum].accent, FADE_DOWN_TIME);
        if (buttonInterruptCheck()) return;

        fadeToColor(swatch[swNum].midtone, swatch[swNum].midtone, FADE_DOWN_TIME);
        if (buttonInterruptCheck()) return;

        fadeToColor(swatch[swNum].contrast, swatch[swNum].contrast, FADE_DOWN_TIME);
        if (buttonInterruptCheck()) return;

        fadeToColor(swatch[swNum].background, swatch[swNum].background, FADE_DOWN_TIME);
    }
}

// -------------------------------------------------------------------------------------
// MARK: glitchLoop
// Advanced neon flicker with 3 different animation patterns selected randomly
void glitchLoop(const uint8_t flickerChance, const uint8_t effectChance, const int duration) {
    // For <duration> milliseconds, both LED segments will either play a special animation or the normal flicker
    unsigned long startTime = millis();
    unsigned long currentTime = millis();
    bool effectTrigger = random(0, 100) < effectChance;
    while (currentTime - startTime < duration) {
        if (buttonInterruptCheck()) return;

        if (effectTrigger) {
            // Apply a special effect
            uint8_t flickerSegment = random(0, numLEDs);
            // Pick a random glitch effect
            switch (random(0, 4)) {
                case 0:
                    glitch1(flickerSegment, 700);
                    break;
                case 1:
                    glitch3(flickerSegment, swatch[swNum].primary, 20, 3);
                    break;
                case 2:
                    glitch4(6, 700);
                    break;
                case 3:
                    glitch5();
                    break;
            }
            currentTime = millis();
        } else {
            // Normal flicker on both segments
            flicker(0, flickerChance, 200, 255);
            flicker(1, flickerChance, 0, 200);
            currentTime = millis();
        }
    }
}

// -------------------------------------------------------------------------------------
// MARK: fadeToColor
void fadeToColor(const uint8_t color1[3], const uint8_t color2[3], const int fadeTime){
    uint8_t startColor[2][3];
    uint8_t output[2][3];

    // Copy handoverColor to startColor
    for (int segment = 0; segment < numLEDs; segment++) {
        for (int pin = 0; pin < 3; pin++) {
            startColor[segment][pin] = handoverColor[segment][pin];
        }
    }

    unsigned long startTime = millis();
    while (millis() - startTime < fadeTime) {
        if (buttonInterruptCheck()) return;

        float fadeRatio = (float)(millis() - startTime) / fadeTime;
            for (int pin = 0; pin < 3; pin++) {
                output[0][pin] = startColor[0][pin] + (color1[pin] - startColor[0][pin]) * fadeRatio;
                output[1][pin] = startColor[1][pin] + (color2[pin] - startColor[1][pin]) * fadeRatio;
            }
        sendToRGB(0, output[0]);
        sendToRGB(1, output[1]);
    }
}

// -------------------------------------------------------------------------------------
// MARK: showColor
void showColor(uint8_t color1[3], uint8_t color2[3], int duration){
    unsigned long startTime = millis();
    while (millis() - startTime < duration) {
        if (buttonInterruptCheck()) return;
        sendToRGB(0, color1);
        sendToRGB(1, color2);
    }
}

// -------------------------------------------------------------------------------------
// MARK: flicker
void flicker(const uint8_t pin, const uint8_t chance, const uint8_t min, const uint8_t max){
    uint8_t outputColor[3];
    uint8_t range = random(min, max);
    gradientPosition(range, outputColor);
    sendToRGB(pin, outputColor);
}

// -------------------------------------------------------------------------------------
// MARK: glitch1
void glitch1(const uint8_t segment, int duration){
    uint8_t otherSegment = 0;
    uint8_t flickerTime = 50;
    unsigned long flashStartTime = millis();
    while (millis() - flashStartTime < duration) {
        if (buttonInterruptCheck()) return;

        if (segment == 0) {
            showColor(swatch[swNum].contrast, swatch[swNum].accent,50);
            showColor(swatch[swNum].contrast, swatch[swNum].background,50);
        } else {
            showColor(swatch[swNum].accent, swatch[swNum].contrast,50);
            showColor(swatch[swNum].background, swatch[swNum].contrast,50);
        }
    }
}

// -------------------------------------------------------------------------------------
// MARK: glitch3
void glitch3(uint8_t segment, uint8_t color2[3], int duration,  uint8_t reps) {
    uint8_t startColor[3] = {handoverColor[segment][0], handoverColor[segment][1], handoverColor[segment][2]};
    uint8_t otherSegment = 0;
    if (segment == 0) {otherSegment = 1;}
    // Hold otherSegment at its handoverColor, and flash segment between startColor and color2 twice
    for (int reps = 0; reps < 3; reps++) {
        if (buttonInterruptCheck()) return;

        if (segment == 0) {
            showColor(startColor, handoverColor[1], duration);
            showColor(color2, handoverColor[1], duration);
        } else {
            showColor(handoverColor[0], startColor, duration);
            showColor(handoverColor[0], color2, duration);
        }
    }
}

// -------------------------------------------------------------------------------------
// MARK: glitch4
void glitch4(uint8_t reps, int duration) {
    uint8_t color[3];
    unsigned long start = millis();
    while (millis() - start < duration) {
        if (buttonInterruptCheck()) return;

        for (uint8_t segment = 0; segment < numLEDs; segment++) {
            gradientPosition(random(1, 255), color);
            for (uint8_t i = 0; i < reps; i++) {
                sendToRGB(segment, color);
                sendToRGB(segment, swatch[swNum].contrast);
            }
        }
    }
}

// -------------------------------------------------------------------------------------
// MARK: glitch5
void glitch5(){
    // Use one of the waveform arrays from waveforms.cpp
    uint8_t waveformIndex = random(0, 2); // Choose between the two available waveforms
    uint8_t outputColor[3];

    // First play through the waveform once
    for (uint8_t i = 0; i < 32; i++) {
        if (buttonInterruptCheck()) return;

        // Get color at this position in the gradient
        gradientPosition(waveform[waveformIndex].waveform[i], outputColor);

        // Show this color on both LEDs briefly
        showColor(outputColor, outputColor, 50);

        // Brief black flash every few steps for a glitchy effect
        if (i % 4 == 0) {
            uint8_t blackColor[3] = {0, 0, 0};
            showColor(blackColor, blackColor, 10);
        }
    }

    // Then do some rapid random jumps between waveform positions
    for (uint8_t i = 0; i < 8; i++) {
        if (buttonInterruptCheck()) return;

        uint8_t randomPos = random(0, 32);
        gradientPosition(waveform[waveformIndex].waveform[randomPos], outputColor);
        showColor(outputColor, outputColor, 30);

        // Brief flashes to black between jumps
        uint8_t blackColor[3] = {0, 0, 0};
        showColor(blackColor, blackColor, 15);
    }

    // End with a final dramatic fade to black
    fadeToColor(swatch[swNum].contrast, swatch[swNum].background, 300);
}

// -------------------------------------------------------------------------------------
// MARK: rapidPulse
void rapidPulse(uint8_t color1[3], uint8_t color2[3], int speed){
    showColor(color1, color1, speed);
    showColor(color2, color2, speed);
}

// -------------------------------------------------------------------------------------
// MARK: fakeMorse
void fakeMorse(uint8_t color1, uint8_t color2, int duration) {
    uint8_t output1[3];
    uint8_t output2[3];
    gradientPosition(color1, output1);
    gradientPosition(color2, output2);
    int interval = 50; // Change every 100ms

    unsigned long start = millis();
    while (millis() - start < duration) {
        if (buttonInterruptCheck()) return;

        int selection = random(0, 3);
        // Set LED colors based on selection
        if (selection == 0) {
            showColor(output1, output2, interval);
        } else if (selection == 1) {
            // Second segment gets color2, first gets color1
            showColor(output2, output1, interval);
        } else {
            // Neither selected, both get color1
            showColor(output1, output1, interval);
        }
    }
}

// -------------------------------------------------------------------------------------
// MARK: swatchPreview
void swatchPreview() {
    const int fadeUpDuration = 50; // 0.2 seconds fade up
    const int fadeDownDuration = 400; // 0.6 seconds fade down
    const int fadeUpSteps = 20; // Steps for fade up
    const int fadeDownSteps = 60; // Steps for fade down

    // Phase 1: Fade UP quickly from dark to bright over 0.2 seconds
    for (int i = 0; i < fadeUpSteps; i++) {
        uint8_t gradientPos = (i * 255) / (fadeUpSteps - 1); // Start at 0 (bright), go to 255 (dark)
        uint8_t outputColor[3];

        gradientPosition(gradientPos, outputColor);

        sendToRGB(0, outputColor);
        sendToRGB(1, outputColor);

        delay(fadeUpDuration / fadeUpSteps);
    }

    // Phase 2: Fade DOWN slowly from bright to dark over 0.6 seconds
    for (int i = 0; i < fadeDownSteps; i++) {
        uint8_t gradientPos = ((fadeDownSteps - 1 - i) * 255) / (fadeDownSteps - 1); // Start at 255 (dark), go to 0 (bright)
        uint8_t outputColor[3];

        gradientPosition(gradientPos, outputColor);

        sendToRGB(0, outputColor);
        sendToRGB(1, outputColor);

        delay(fadeDownDuration / fadeDownSteps);
    }

    // Reset the flag
    swatchPreviewActive = false;
}

// -------------------------------------------------------------------------------------
// MARK: animationPreview
void animationPreview() {
    const int flashDuration = 20; // Quick flash duration in ms
    const int numFlashes = 3; // Number of flashes to indicate mode change

    // Flash the primary color quickly to indicate animation mode change
    for (int i = 0; i < numFlashes; i++) {
        // Bright flash
        sendToRGB(0, swatch[swNum].primary);
        sendToRGB(1, swatch[swNum].primary);
        delay(flashDuration);

        // Dark flash
        sendToRGB(0, swatch[swNum].background);
        sendToRGB(1, swatch[swNum].background);
        delay(flashDuration);
    }

    // Reset the flag
    animationPreviewActive = false;
}

// -------------------------------------------------------------------------------------
// MARK: brightnessAdjustmentMode
void brightnessAdjustmentMode() {
    const int cycleDuration = 4000; // 4 seconds total
    const int stepDuration = 100; // Update every 100ms

    unsigned long modeStartTime = millis();

    // Use white color for brightness display
    uint8_t whiteColor[3] = {255, 255, 255};

    while (brightnessAdjustMode && digitalRead(colorBtn) == LOW) {
        unsigned long elapsedTime = millis() - modeStartTime;

        // Simple triangle wave for brightness cycling
        float cyclePos = (float)(elapsedTime % cycleDuration) / cycleDuration;
        float brightnessRatio = (cyclePos < 0.5) ? cyclePos * 2 : 2 - (cyclePos * 2);

        // Map to brightness range
        currentBrightness = minBrightness + (maxBrightness - minBrightness) * brightnessRatio;

        // Display white at current brightness on both segments
        // Use direct LED control to avoid button checking interference
        for (int i = 0; i < 10; i++) { // Display multiple times per step for stability
            sendToRGB(0, whiteColor);
            sendToRGB(1, whiteColor);
            delay(stepDuration / 10);
        }
    }

    // Reset mode flag
    brightnessAdjustMode = false;
}

bool buttonInterruptCheck() {
    if (swatchPreviewActive || animationPreviewActive || brightnessAdjustMode) {
        return true; // Interrupt detected
    }
    return false; // No interrupt
}
