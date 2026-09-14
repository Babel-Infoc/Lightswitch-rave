#include "swatches.h"
#include <Arduino.h>

// Define macro to convert rgb(r, g, b) to {r, g, b} for VSCode color tags
#define rgb(r, g, b) {r, g, b}
#define rgba(r, g, b, a) {r, g, b}

// Initialise the swatch array index
uint8_t swNum = 0;

// Master array of swatches
// All color values should have at least one channel at 255
// to ensure that subsequent brightness functions are performed correctly
/*
    primary     = 0
    accent      = 1
    midtone     = 2
    contrast    = 3
    background  = 4
*/
const swatchArray swatch[] = {
    {
        // 0 Neurotoxin
        rgb(200, 255, 0),
        rgb(180, 255, 0),
        rgb(170, 255, 0),
        rgb(170, 0, 255),
        rgb(200, 0, 255),
    },{
        // 1 Hazard pay
        rgb(255, 250, 0),
        rgb(255, 200, 0),
        rgb(255, 150, 0),
        rgb(255, 100, 0),
        rgb(255, 50, 0),
    },{
        // 2 Red mist
        rgb(255, 0, 255),
        rgb(255, 0, 200),
        rgb(255, 0, 150),
        rgb(255, 0, 100),
        rgb(255, 0, 50),
    },{
        // 3 Arasaka
        rgb(255, 255, 255),
        rgb(255, 180, 180),
        rgb(255, 120, 120),
        rgb(255, 60, 60),
        rgb(255, 0,  0),
    },{
        // 4 Acid spill
        rgb(0, 255, 255),
        rgb(0, 255, 200),
        rgb(0, 255, 150),
        rgb(0, 255, 100),
        rgb(0, 255, 50),
    },{
        // 5 Mox
        rgb(255, 255, 0),
        rgb(200, 255, 0),
        rgb(150, 255, 0),
        rgb(100, 255, 0),
        rgb(50,  255, 0),
    },{
        // 6 Isotope
        rgb(255, 255, 0),
        rgb(120, 255, 0),
        rgb(0,   255, 0),
        rgb(0,   255, 120),
        rgb(0,   255, 255),
    },{
        // 7 Static romance
        rgb(255, 0, 255),
        rgb(200, 0, 255),
        rgb(150, 0, 255),
        rgb(100, 0, 255),
        rgb(50,  0, 255),
    },{
        // 8 Wintermute
        rgb(0, 255, 255),
        rgb(0, 200, 255),
        rgb(0, 150, 255),
        rgb(0, 100, 255),
        rgb(0,  50, 255),
    },{
        // 9 Corpo
        rgb(200, 150, 255),
        rgb(50, 0, 255),
        rgb(150, 0, 255),
        rgb(200, 0, 255),
        rgb(200, 0, 255),
    },{
        // 10 Mono
        rgb(255, 255, 255),
        rgb(255, 255, 255),
        rgb(255, 255, 255),
        rgb(255, 255, 255),
        rgb(255, 255, 255),
    },{
        // 11 Heat treated
        rgb(255, 220, 100),
        rgb(255, 100, 0),
        rgb(255, 0, 100),
        rgb(150, 0, 255),
        rgb(0, 0, 255)
    },{
        // 12 Stand back, citizen
        rgb(255, 0, 0),
        rgb(255, 0, 0),
        rgb(0, 0, 255),
        rgb(0, 0, 255),
        rgb(0, 0, 255),
    },{
        // 13 Brandi
        rgb(255, 200, 255),
        rgb(255, 150, 255),
        rgb(255, 100, 255),
        rgb(255, 50, 255),
        rgb(255, 0, 255),
    },{
        // 14 Cryo
        rgb(255, 255, 255),
        rgb(200, 200, 255),
        rgb(150, 150, 255),
        rgb(100, 100, 255),
        rgb(50, 50, 255),
    },{
        // 15 Sax solo
        rgb(0,    255,  255),
        rgb(0,    180,  255),
        rgb(100,  100,  255),
        rgb(255,  0,    180),
        rgb(255,  0,    255),
    },{
        // 16 Wastelander
        rgb(200, 200, 255),
        rgb(0, 150, 255),
        rgb(100, 100, 255),
        rgb(255, 150, 0),
        rgb(255, 100, 0),
    },{
        // 17 Nerve damage
        rgb(0, 255, 200),
        rgb(0, 255, 150),
        rgb(100, 255, 100),
        rgb(255, 0, 255),
        rgb(255, 0, 200),
    },{
        // 18 Villa Straylight
        rgb(210,  200,  255),
        rgb(200,  180,  255),
        rgb(190,  160,  255),
        rgb(180,  140,  255),
        rgb(170,  120,  255),
    },{
        // 19 Megawatt
        rgb(255, 220, 100),
        rgb(255, 0, 255),
        rgb(0, 150, 255),
        rgb(100, 100, 255),
        rgb(0, 255, 255),
    },{
        // 20 Marathon
        rgb(255, 0,    150),
        rgb(100, 255,  0),
        rgb(0,   255,  255),
        rgb(0,   180,  255),
        rgb(0,   100,  255),
    },{
        // 21 Crater
        rgb(255, 150, 0),
        rgb(100, 255,  0),
        rgb(255, 0,    255),
        rgb(0,  0,  255),
        rgb(255,  0,  0),
    }
};

// Calculate the number of swatches automatically based on array size
const uint8_t numSwatches = sizeof(swatch) / sizeof(swatch[0]);

// MARK: gradientPosition ------------------------------------------------------------------------------------------------
// Calculates a gradient rgb color between all 5 colors in the current swatch
// position: 255 = background color, 0 = primary color
void gradientPosition(uint8_t position, uint8_t output[3]) {
    // Create an rgb_t object to return
    uint8_t segment = position >> 6;    // Divide by 64 (4 segments of 64 steps each) (inv_position >> 6)
    uint8_t step = position & 0x3F;     // Get remainder (0-63) (inv_position & 0x3F)
    // Clamp segment to valid range (segment > 3)
    if (segment > 3) segment = 3;
    // Calculate pointers to start and end colors (startColor, endColor)
    const uint8_t* startColor = &swatch[swNum].primary[0] + (4 - segment) * 3;
    const uint8_t* endColor = &swatch[swNum].primary[0] + (3 - segment) * 3;

    // Calculate interpolation factor (0-63 to 0-255)
    uint8_t alpha = step << 2;  // Multiply by 4 to scale 0-63 to 0-252

    // Linear interpolation between colors
    output[0] = ((uint16_t)endColor[0] * alpha + (uint16_t)startColor[0] * (255 - alpha)) >> 8;
    output[1] = ((uint16_t)endColor[1] * alpha + (uint16_t)startColor[1] * (255 - alpha)) >> 8;
    output[2] = ((uint16_t)endColor[2] * alpha + (uint16_t)startColor[2] * (255 - alpha)) >> 8;
}
