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
	~MemoryMap();

	MemoryRegion* GetRegionFromAddress(uint16_t address);

	void Write8(uint16_t address, uint8_t data);
	void Write16(uint16_t address, uint16_t data);
	uint8_t Read8(uint16_t address);
	uint16_t Read16(uint16_t address);
};
