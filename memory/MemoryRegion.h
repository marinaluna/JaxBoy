#pragma once
#include "../util/Macros.h"

#include <cstdint>


class MemoryRegion
{
protected:
    uint8_t* memory;
    uint16_t offset;

public:
    MemoryRegion(uint16_t offset, uint16_t size);
    ~MemoryRegion();

    virtual void write8(uint16_t address, uint8_t data);
    virtual void write16(uint16_t address, uint16_t data);
    virtual uint8_t read8(uint16_t address);
    virtual uint16_t read16(uint16_t address);
    virtual void memcpy(uint16_t offset, void* dataptr, uint16_t bytes);
};

class ForbiddenRegion : public MemoryRegion
{
public:
    ForbiddenRegion(uint16_t offset, uint16_t size) : MemoryRegion(offset, size){}

    void write8(uint16_t address, uint8_t data) {}
    void write16(uint16_t address, uint8_t data) {}
    uint8_t read8(uint16_t address) { return 0xFF; }
    uint16_t read16(uint16_t address) { return 0xFFFF; }
};
