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
    },
    /*{
        // Green
        rgb(238, 255, 0),
        rgb(115, 255, 0),
        rgb(0, 255, 179),
        rgb(0,    100,      0),
        rgb(0,    50,       0),
    },
    {   // Assegai
        rgb(255, 111, 0),
        rgb(255, 225, 0),
        rgb(0, 225, 255),
        rgb(166, 0, 255),
        rgb(92, 222, 0),
    },
    {   // Wintermute
        rgb(0,      255,    255),
        rgb(0, 208, 255),
        rgb(255, 0, 68),
        rgb(128, 0, 219),
        rgb(85,     0,      141),
    },
    {   // Classic synthwave
        rgb(255,    0,      255),
        rgb(255, 0, 230),
        rgb(0, 255, 242),
        rgb(0, 233, 209),
        rgb(0,      0,      0),
    },
    {   // Redshift
        rgb(130, 108, 255),
        rgb(68, 0, 255),
        rgb(204, 0, 255),
        rgb(204, 0, 0),
        rgb(102,    0,      26),
    },
    {   // Chromatic abberation
        rgb(255, 112, 112),
        rgb(255,    255,    255),
        rgb(65, 122, 255),
        rgb(130, 130, 130),
        rgb(20,     20,     20),
    },*/
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
