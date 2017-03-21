#pragma once
#include <cstdint>

class Rom
{
    struct Header
    {
        char Name[16];
        char Manufacturer[4];
        bool UsesSGBFeatures;
        uint8_t CartType;
        uint8_t RomSize;
        uint8_t RamSize;
        bool International;
        uint8_t Licensee;
        uint8_t VersionCode;
    };
    Header RomInfo;

    uint8_t InterruptVectors[0x100];

    const char* rom_name;
public:

    Rom(const char* rom_name, uint8_t* rom_bytes);

    uint8_t* GetInterruptVectorsPtr();
    /*int getSizeInBytes() const;
    uint16_t getStartAddress() const;
    void printDebugInfo() const;*/
};
