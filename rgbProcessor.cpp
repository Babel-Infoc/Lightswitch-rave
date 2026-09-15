#include <Arduino.h>
#include "types.h"
#include "swatches.h"
#include "flashStorage.h"

// Initialize the buttons
uint8_t colorButtonLastState = HIGH;
uint8_t animButtonLastState = HIGH;
uint8_t colorIndex = 0;

uint8_t debounceStart = 0;
// Flag to signal animation interruption
bool animInterrupt = false;

// Replace std::array with plain C arrays to save significant flash memory
float tuneRatio[3] = {1.0, 1.0, 1.0};
uint8_t handoverColor[2][3] = {{0, 0, 0}, {0, 0, 0}};

// Reference to the led array defined in the main sketch
extern ledSegment led[2];

// MARK: Gamma Correction ------------------------------
const uint8_t gamma8[] = {
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
    float rRatio, gRatio, bRatio;
    float maxLum = max(max(red.luminance, green.luminance), blue.luminance);
    rRatio    = (red.luminance / maxLum) / red.mA;
    gRatio    = (green.luminance / maxLum) / green.mA;
    bRatio    = (blue.luminance / maxLum) / blue.mA;
    float tuneRatio[] = {rRatio, gRatio, bRatio};

    // Get the highest Ratio value, adjust it up to 1, and adjust the other two ratios up at the same rate
    float maxRatio = max(rRatio, max(gRatio, bRatio));
    tuneRatio[0] = maxRatio / rRatio;
    tuneRatio[1] = maxRatio / gRatio;
    tuneRatio[2] = maxRatio / bRatio;
}

// MARK: Button Handling ------------------------------
void checkButtons() {
    if (debounceStart > 0) {
        debounceStart--;
    } else {
        // Reset the interrupt flag
        animInterrupt = false;
        // Check color button
        uint8_t colorButtonState = digitalRead(colorBtn);
        if (colorButtonState != colorButtonLastState) {
            if (colorButtonState == LOW) {
                swNum = (swNum + 1) % numSwatches;

                // Save the new swatch number to flash
                saveSettingsToFlash(swNum, animIndex);
            }
            colorButtonLastState = colorButtonState;
        }

        // Check animation button
        uint8_t animButtonState = digitalRead(animBtn);
        if (animButtonState != animButtonLastState) {
            if (animButtonState == LOW) {
                animIndex = (animIndex + 1) % numAnimations;
                // Set the interrupt flag to break current animation
                animInterrupt = true;

                // Save the new animation index to flash
                saveSettingsToFlash(swNum, animIndex);
            }
            animButtonLastState = animButtonState;
        }

        // Restart the debounce timer
        debounceStart = 300;
    }
}

// MARK: Light sensor ------------------------------

// MARK: RGB Processing ------------------------------
// Function to process the raw RGB values to accurate and consistent luminosity and hue
void sendToRGB(const uint8_t segment, const uint8_t rgbValue[3]) {
    float rRatio, gRatio, bRatio;
    int tunedRGB[3];

    // Write the end color to the handover color matching the led segment
    // If segment 0 is updated, also mirror it to segment 1 so channel 2 matches channel 1.
    for (int i = 0; i < 3; i++) {
        handoverColor[segment][i] = rgbValue[i];
        if (segment == 0) {
            handoverColor[1][i] = rgbValue[i];
        }
    }

    // Calculate the brightness-adjusted and gamma-corrected values using global tuneRatio
    for (uint8_t pin = 0; pin < 3; pin++) {
        tunedRGB[pin] = gamma8[(int)(rgbValue[pin] * tuneRatio[pin] * maxBrightness)];
    }

    // Determine which segments to update: always update the requested segment,
    // and when segment 0 is written also update segment 1 to mirror it.
    uint8_t targets[2];
    uint8_t targetCount = 1;
    targets[0] = segment;
    if (segment == 0) {
        targets[1] = 1;
        targetCount = 2;
    }

    // Set the pin states for each target segment based on the tuned RGB values
    for (uint8_t t = 0; t < targetCount; t++) {
        uint8_t seg = targets[t];
        for (int brightness = 0; brightness < 100; brightness++) {
            digitalWrite(led[seg].red, brightness < tunedRGB[0] ?      LOW : HIGH);
            digitalWrite(led[seg].green, brightness < tunedRGB[1] ?    LOW : HIGH);
            digitalWrite(led[seg].blue, brightness < tunedRGB[2] ?     LOW : HIGH);
        }
    }

    // Check buttons once per frame, with debounce handling
    checkButtons();
    // Slow down to prevent excessive CPU usage
    delay(slowDown);
}
