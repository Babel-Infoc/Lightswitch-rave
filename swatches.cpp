#include "swatches.h"
#include <Arduino.h>

// Define macro to convert rgb(r, g, b) to {r, g, b}
#define rgb(r, g, b) {r, g, b}
#define rgba(r, g, b, a) {r, g, b}

// Initialise the swatch array index
uint8_t swNum = 0;

/*
    primary
    accent
    midtone
    contrast
    background
*/
// Master array of swatches
swatchArray swatch[] = {
    {
        // Hazard pay
        rgba(255,   200,    0,      1),
        rgba(255,   150,    0,      1),
        rgba(200,   100,    0,      1),
        rgba(100,   50,     0,      1),
        rgba(50,    0,      0,      1),
    },
    {
        // Hazard pay
        rgb(255, 171, 93),
        rgb(255, 30, 0),
        rgb(255, 30, 0),
        rgb(100,    0,      0),
        rgb(0, 0, 0),
    },
    {
        // VHS pink
        rgba(255, 208, 243, 1),
        rgba(255, 166, 240, 1),
        rgba(105, 8, 143, 1),
        rgba(24, 7, 85, 1),
        rgb(0, 0, 0),
    },
    {   // Orange
        rgba(255, 187, 132, 1),
        rgb(255, 150, 64),
        rgba(255, 115, 0, 1),
        rgba(148, 67, 0, 1),
        rgba(0, 0, 0, 1),
    },
    {
        // Blue
        rgb(225, 0, 255),
        rgb(98, 0, 255),
        rgb(0, 119, 255),
        rgb(0,    0,      100),
        rgb(0,    0,      50),
    },
    {   // Chemical spill
        rgb(166,    255,    0),
        rgb(145,    255,    0),
        rgb(136, 0, 255),
        rgb(107, 0, 230),
        rgb(65,     0,      131),
    },
    {   // Brandi
        rgb(255,    0,      255),
        rgb(255,    0,      200),
        rgb(255,    0,      150),
        rgb(218, 0, 109),
        rgb(100,    0,      50),
    },
    {   // Villa Straylight
        rgb(186, 143, 255),
        rgb(187, 142, 255),
        rgb(149, 111, 255),
        rgb(60, 0, 199),
        rgb(40,     40,     50),
    },
    {   // Strawbs
        rgb(166,    255,    0),
        rgb(145,    255,    0),
        rgb(255, 0, 200),
        rgb(230, 0, 88),
        rgb(131, 0, 50),
    },{
        // Hazard pay
        rgba(255,   200,    0,      1),
        rgba(255,   150,    0,      1),
        rgba(200,   100,    0,      1),
        rgba(100,   50,     0,      1),
        rgba(50,    0,      0,      1),
    },{
        // Red mist
        rgba(255,   150,    150,    1),
        rgba(255,   0,      150,    1),
        rgba(200,   0,      100,    1),
        rgba(150,   0,      50,     1),
        rgba(0,     0,      0,      1),
    },{
        // Arasaka
        rgba(255,   0,      0,      1),
        rgba(200,   0,      0,      1),
        rgba(150,   0,      0,      1),
        rgba(100,   0,      0,      1),
        rgba(0,     0,      0,      1),
    },{
        // Acid spill
        rgba(0,     255,    200,    1),
        rgba(0,     255,    150,    1),
        rgba(0,     200,    100,    1),
        rgba(0,     100,    50,     1),
        rgba(0,     0,      0,      1),
    },{
        // Mox
        rgba(150,   255,    150,    1),
        rgba(150,   255,    0,      1),
        rgba(100,   200,    0,      1),
        rgba(50,    150,    0,      1),
        rgba(0,     0,      0,      1),
    },{
        // Static romance
        rgba(200,   0,      255,    1),
        rgba(150,   0,      255,    1),
        rgba(100,   0,      200,    1),
        rgba(50,    0,      100,    1),
        rgba(0,     0,      0,      1),
    },{
        // Purple
        rgba(100,   0,      255,    1),
        rgba(80,    0,      255,    1),
        rgba(60,    0,      200,    1),
        rgba(40,    0,      100,    1),
        rgba(0,     0,      0,      1),
    },{
        // Corpo
        rgba(150,   150,    255,    1),
        rgba(0,     150,    255,    1),
        rgba(0,     100,    200,    1),
        rgba(0,     50,     150,    1),
        rgba(0,     0,      0,      1),
    },{
        // Radioactive
        rgba(255,   255,    150,    1),
        rgba(255,   255,    0,      1),
        rgba(200,   250,    0,      1),
        rgba(120,   100,    0,      1),
        rgba(100,   50,     0,      1),
    },{
        // Brandi
        rgba(255,   0,      255,    1),
        rgba(200,   0,      200,    1),
        rgba(150,   0,      150,    1),
        rgba(100,   0,      100,    1),
        rgba(50,    0,      50,     1),
    },{
        // Wintermute
        rgba(0,   255,      255,    1),
        rgba(0,   200,      200,    1),
        rgba(0,   150,      150,    1),
        rgba(0,   100,      100,    1),
        rgba(0,    50,      50,     1),
    },{
        // Mono
        rgba(255,   255,    255,    1),
        rgba(200,   200,    200,    1),
        rgba(150,   150,    150,    1),
        rgba(100,   100,    100,    1),
        rgba(10,    10,     10,     1),
    },
};

// Calculate the number of swatches automatically based on array size
uint8_t numSwatches = sizeof(swatch) / sizeof(swatch[0]);

// Colors used for the bootup animation
const uint8_t bootswatch[5][3] = {
    rgb(255, 220, 106),
    rgb(209, 0, 52),
    rgb(75, 0, 130),
    rgb(0, 0, 97),
    rgb(0, 0, 0)
};

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
