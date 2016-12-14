#include "MemoryMap.h"


MemoryMap::MemoryMap(size_t size)
{
	memory = (uint8_t*) ::operator new(size);
}

void MemoryMap::write8(uint16_t address, uint8_t data)
{
	memory[address] = data;
}

void MemoryMap::write16(uint16_t address, uint16_t data)
{
	memory[address] = (data & 0xFF00) >> 8;
	memory[address + 1] = data & 0x00FF;
}

uint8_t MemoryMap::read8(uint16_t address)
{
	return memory[address];
}

uint16_t MemoryMap::read16(uint16_t address)
{
	return (memory[address] << 8) | (memory[address + 1]);
}
