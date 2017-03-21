#pragma once
#include <cstdlib>

class MemoryRegion;
class MemoryMap
{
    MemoryRegion* program_rom_region;
    MemoryRegion* video_ram_region;
    MemoryRegion* external_ram_region;
    MemoryRegion* internal_ram_region;
    MemoryRegion* forbidden1_region;
    MemoryRegion* oam_region;
    MemoryRegion* forbidden2_region;
    MemoryRegion* mmio_region;
    MemoryRegion* high_ram_region;

public:
    MemoryMap();
    ~MemoryMap();

    MemoryRegion* GetRegionFromAddress(uint16_t address);

    void IOSpecialWrite(uint16_t address, uint8_t data);
    bool IOSpecialRead(uint16_t address, uint8_t& retval);

    void Write8(uint16_t address, uint8_t data);
    void Write16(uint16_t address, uint16_t data);
    uint8_t Read8(uint16_t address);
    uint16_t Read16(uint16_t address);
};
