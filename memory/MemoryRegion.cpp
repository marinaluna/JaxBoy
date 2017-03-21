#include <cstring> // for memcpy, memset

#include "MemoryRegion.h"


MemoryRegion::MemoryRegion(uint16_t offset, uint16_t size)
:offset(offset)
{
    memory = new uint8_t[size];
    memset((void*) memory, 0x00, size);
}

MemoryRegion::~MemoryRegion()
{
    if(memory != nullptr)
        delete memory;
}

void MemoryRegion::memcpy(uint16_t offset, void* dataptr, uint16_t bytes)
{
    ::memcpy((void*) (memory + offset), dataptr, bytes);
}

void MemoryRegion::write8(uint16_t address, uint8_t data)
{
    memory[address - offset] = data;
}

void MemoryRegion::write16(uint16_t address, uint16_t data)
{
    memory[address - offset] = data & 0xFF;
    memory[address + 1 - offset] = (data & 0xFF00) >> 8;
}

uint8_t MemoryRegion::read8(uint16_t address)
{
    return memory[address - offset];
}

uint16_t MemoryRegion::read16(uint16_t address)
{
    return (memory[address - offset]) | (memory[address + 1 - offset] << 8);
}
