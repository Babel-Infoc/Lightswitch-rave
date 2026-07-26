#include "flashStorage.h"
#include "swatches.h"  // Include this to access numSwatches

// Signature value to identify valid settings
#define SETTINGS_SIGNATURE 0xA5

// Define the flash page to use for settings storage
// CH32V003 has 16KB flash total, program code typically takes < 10KB
// Use a page near the end but leave some safety margin
// Flash page size is 64 bytes on CH32V003, not 1KB
#define FLASH_PAGE_SIZE 64
// Use address at 14KB mark (0x08003FC0), well away from program code
#define FLASH_SETTINGS_PAGE_ADDR 0x08003FC0

// Calculate checksum for settings
uint8_t calculateChecksum(const FlashSettings* settings) {
    uint8_t sum = 0;
    const uint8_t* bytes = (const uint8_t*)settings;
    // Sum all bytes except the checksum field itself
    for (uint8_t i = 0; i < offsetof(FlashSettings, checksum); i++) {
        sum += bytes[i];
    }
    return ~sum; // Use one's complement for better error detection
}

// Load settings from flash memory
bool loadSettingsFromFlash(uint8_t* swatchNum, float* brightness, uint8_t* animationMode) {
    // Point to the flash memory where settings are stored
    const FlashSettings* storedSettings = (const FlashSettings*)FLASH_SETTINGS_PAGE_ADDR;

    // Verify signature and checksum
    if (storedSettings->signature == SETTINGS_SIGNATURE) {
        uint8_t checksum = calculateChecksum(storedSettings);
        if (checksum == storedSettings->checksum) {
            // Valid settings found, but also check range validity
            if (storedSettings->swatchNumber < numSwatches) {
                // Settings are valid, load them
                *swatchNum = storedSettings->swatchNumber;
                // Convert brightness from 0-255 scale back to 0.0-1.0 scale
                *brightness = (float)storedSettings->brightness / 255.0f;
                *animationMode = storedSettings->animationMode;
                return true;
            }
        }
    }

    // No valid settings found or settings out of range
    return false;
}

// Save settings to flash memory
bool saveSettingsToFlash(uint8_t swatchNum, float brightness, uint8_t animationMode) {
    // Don't attempt to save if values are out of expected range
    if (swatchNum >= numSwatches || brightness < 0.0f || brightness > 1.0f) {
        return false;
    }

    // Prepare settings structure
    FlashSettings newSettings;
    newSettings.signature = SETTINGS_SIGNATURE;
    newSettings.swatchNumber = swatchNum;
    // Convert brightness from 0.0-1.0 scale to 0-255 scale for storage
    newSettings.brightness = (uint8_t)(brightness * 255.0f);
    newSettings.animationMode = animationMode;
    newSettings.checksum = calculateChecksum(&newSettings);

    // Unlock flash for writing
    FLASH_Unlock();

    // Erase the settings page
    FLASH_Status status = FLASH_ErasePage(FLASH_SETTINGS_PAGE_ADDR);
    if (status != FLASH_COMPLETE) {
        FLASH_Lock();
        return false;
    }

    // Write settings to flash - CH32V003 requires 32-bit aligned writes
    uint32_t* sourceData = (uint32_t*)&newSettings;
    uint32_t flashAddr = FLASH_SETTINGS_PAGE_ADDR;
    bool writeSuccess = true;

    // Only write as many 32-bit words as needed for our settings structure
    for (uint8_t i = 0; i < (sizeof(FlashSettings) + 3) / 4; i++) {
        status = FLASH_ProgramWord(flashAddr, *sourceData);
        if (status != FLASH_COMPLETE) {
            writeSuccess = false;
            break;
        }
        flashAddr += 4;
        sourceData++;
    }

    // Lock flash again
    FLASH_Lock();

    if (!writeSuccess) {
        return false;
    }

    // Verify the written data
    const FlashSettings* storedSettings = (const FlashSettings*)FLASH_SETTINGS_PAGE_ADDR;
    if (storedSettings->swatchNumber == swatchNum &&
        storedSettings->brightness == (uint8_t)(brightness * 255.0f) &&
        storedSettings->signature == SETTINGS_SIGNATURE &&
        storedSettings->checksum == calculateChecksum(storedSettings)) {
        return true;
    }

    return false;
}
