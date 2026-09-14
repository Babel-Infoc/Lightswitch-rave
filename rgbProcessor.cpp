#include <Arduino.h>
#include "types.h"
#include "swatches.h"
#include "flashStorage.h"

// Initialize the buttons
uint8_t colorButtonLastState = HIGH;
uint8_t animButtonLastState = HIGH;
uint8_t colorIndex = 0;

uint8_t debounceStart = 0;

// Animation mode (0 = glitchLoop, 1 = other animations...)
uint8_t animationMode = 0;
const uint8_t numAnimationModes = 4; // Currently only glitchLoop, expand this as you add more

// Swatch preview animation flag
bool swatchPreviewActive = false;

// Animation preview flag
bool animationPreviewActive = false;

// Brightness adjustment mode variables
unsigned long buttonPressStartTime = 0;
bool buttonHeldFor2Seconds = false;

// Replace std::array with plain C arrays to save significant flash memory
float tuneRatio[3] = {1.0, 1.0, 1.0};
uint8_t handoverColor[2][3] = {{0, 0, 0}, {0, 0, 0}};

// Reference to the led array and buttons defined in the main sketch
extern ledSegment led[2];
extern uint8_t colorBtn;
extern uint8_t animBtn;
extern float currentBrightness;

// MARK: Gamma Correction ------------------------------
const uint8_t PROGMEM gamma8[] = {
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,  2,
    2,  2,  2,  3,  3,  3,  3,  3,  3,  3,  4,  4,  4,  4,  4,   5,
    5,  5,  5,  6,  6,  6,  6,  7,  7,  7,  7,  8,  8,  8,  9,   9,
    9, 10, 10, 10, 11, 11, 11, 12, 12, 13, 13, 13, 14, 14, 15,  15,
   16, 16, 17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23,  24,
   24, 25, 25, 26, 27, 27, 28, 29, 29, 30, 31, 32, 32, 33, 34,  35,
   35, 36, 37, 38, 39, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48,  49,
   50, 50, 51, 52, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64,  66,
   67, 68, 69, 70, 72, 73, 74, 75, 77, 78, 79, 81, 82, 83, 85,  86,
   87, 89, 90, 92, 93, 95, 96, 98, 99,101,102,104,105,107,109, 110,
  112,114,115,117,119,120,122,124,126,127,129,131,133,135,137, 138,
  140,142,144,146,148,150,152,154,156,158,160,162,164,167,169, 171,
  173,175,177,180,182,184,186,189,191,193,196,198,200,203,205, 208,
  210,213,215,218,220,223,225,228,231,233,236,239,241,244,247, 249,
  252,255 };

// MARK: Luminance Calculation ------------------------------
// Function to calculate luminance ratios for consistent color output
void calculateLuminance() {
    // Calculate the effective luminance per mA for each LED
    float rEfficiency = red.luminance / red.mA;
    float gEfficiency = green.luminance / green.mA;
    float bEfficiency = blue.luminance / blue.mA;

    // Find the most efficient LED (highest luminance per mA)
    float maxEfficiency = max(rEfficiency, max(gEfficiency, bEfficiency));

    // Scale all LEDs DOWN from the most efficient one
    // This ensures no LED is driven above its baseline
    tuneRatio[0] = rEfficiency / maxEfficiency; // Red ratio (≤ 1.0)
    tuneRatio[1] = gEfficiency / maxEfficiency; // Green ratio (≤ 1.0)
    tuneRatio[2] = bEfficiency / maxEfficiency; // Blue ratio (≤ 1.0)
}

// MARK: Button Handling ------------------------------
void checkButtons() {
    // Don't process button events while a preview animation is playing.
    // sendToRGB calls checkButtons on every PWM frame, so without this guard
    // a button release detected mid-animation would advance swNum a second time.
    if (swatchPreviewActive || animationPreviewActive) return;

    if (debounceStart > 0) {
        debounceStart--;
    } else {
        // Check color button
        uint8_t colorButtonState = digitalRead(colorBtn);

        if (colorButtonState != colorButtonLastState) {
            if (colorButtonState == LOW) {
                // Button just pressed - start timing
                buttonPressStartTime = millis();
                buttonHeldFor2Seconds = false;
            } else {
                // Button just released
                if (!buttonHeldFor2Seconds) {
                    // Short press - change swatch
                    swNum = (swNum + 1) % numSwatches;
                    swatchPreviewActive = true;
                    // Save the new swatch to flash
                    saveSettingsToFlash(swNum, currentBrightness, animationMode);
                } else {
                    // Long press was released - exit brightness mode
                    brightnessAdjustMode = false;
                    // Save brightness setting to flash
                    saveSettingsToFlash(swNum, currentBrightness, animationMode);
                }
                buttonHeldFor2Seconds = false;
            }
            colorButtonLastState = colorButtonState;
        } else if (colorButtonState == LOW && !buttonHeldFor2Seconds) {
            // Button is being held - check if trigger time has passed
            if (millis() - buttonPressStartTime >= brightnessModeTriggerTime) {
                buttonHeldFor2Seconds = true;
                brightnessAdjustMode = true;
            }
        }

        // Check animation button (skipped when no button is wired to this config)
        uint8_t animButtonState = digitalRead(animBtn);

        if (animButtonState != animButtonLastState) {
            if (animButtonState == LOW) {
                // Button just pressed - cycle to next animation mode
                animationMode = (animationMode + 1) % numAnimationModes;
                animationPreviewActive = true;
                // Save the new animation mode to flash
                saveSettingsToFlash(swNum, currentBrightness, animationMode);
            }
            animButtonLastState = animButtonState;
        }

        // Restart the debounce timer
        debounceStart = 20; // Reduced for more responsive long press detection
    }
}

// MARK: Light sensor ------------------------------

// MARK: RGB Processing ------------------------------
// Function to process the raw RGB values to accurate and consistent luminosity and hue
void sendToRGB(const uint8_t segment, const uint8_t rgbValue[3]) {
    float rRatio, gRatio, bRatio;
    int tunedRGB[3];

    // Write the end color to the handover color matching the led segment
    for (int i = 0; i < 3; i++) {
        handoverColor[segment][i] = rgbValue[i];
    }

    // Calculate the brightness-adjusted and gamma-corrected values using global tuneRatio
    for (uint8_t pin = 0; pin < 3; pin++) {
        // Calculate the final RGB value with bounds checking to prevent overflow
        float adjustedValue = rgbValue[pin] * tuneRatio[pin] * currentBrightness;
        // Constrain to valid gamma table range (0-255)
        int constrainedValue = constrain((int)adjustedValue, 0, 255);
        tunedRGB[pin] = pgm_read_byte(&gamma8[constrainedValue]);
    }

    // Set the pin states based on the tuned RGB values
    for (int brightness = 0; brightness < 100; brightness++) {
        digitalWrite(led[segment].red, brightness < tunedRGB[0] ?      LOW : HIGH);
        digitalWrite(led[segment].green, brightness < tunedRGB[1] ?    LOW : HIGH);
        digitalWrite(led[segment].blue, brightness < tunedRGB[2] ?     LOW : HIGH);
    }

    // Check buttons once per frame, with debounce handling
    checkButtons();
    // Slow down to prevent excessive CPU usage
    delay(slowDown);
}
