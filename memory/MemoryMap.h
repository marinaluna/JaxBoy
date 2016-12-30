#pragma once

#include <cstdlib>

class MemoryRegion;

class MemoryMap
{
	MemoryRegion* program_rom_area;
	MemoryRegion* video_ram_area;
	MemoryRegion* external_ram_area;
	MemoryRegion* internal_ram_area;
	MemoryRegion* oam_area;
	MemoryRegion* mmio_area;
	MemoryRegion* high_ram_area;

public:
	MemoryMap(size_t size);

	MemoryRegion* GetRegionFromAddress(uint16_t address);

	void write8(uint16_t address, uint8_t data);
	void write16(uint16_t address, uint16_t data);
	uint8_t read8(uint16_t address);
	uint16_t read16(uint16_t address);
};
