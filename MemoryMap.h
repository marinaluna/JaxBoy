#pragma once

#include "defs.h"


class MemoryMap
{
	uint8_t* memory;

public:
	MemoryMap(size_t size);

	void write8(uint16_t address, uint8_t data);
	void write16(uint16_t address, uint16_t data);
	uint8_t read8(uint16_t address);
	uint16_t read16(uint16_t address);
};
