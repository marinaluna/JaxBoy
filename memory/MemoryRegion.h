#pragma once
#include "../util/Macros.h"

#include <cstdint>


class MemoryRegion
{
protected:
	uint8_t* memory;
	uint16_t offset;

	MemoryRegion(uint16_t offset, uint16_t size);

public:
	~MemoryRegion();
	
	void memcpy(uint16_t offset, void* dataptr, uint16_t bytes);

	virtual void write8(uint16_t address, uint8_t data);
	virtual void write16(uint16_t address, uint16_t data);
	virtual uint8_t read8(uint16_t address);
	virtual uint16_t read16(uint16_t address);
};

class PGROM : public MemoryRegion
{
public:
	PGROM(uint16_t size) : MemoryRegion(PGROM_OFFSET, size){}
};
class VRAM : public MemoryRegion
{
public:
	VRAM(uint16_t size) : MemoryRegion(VRAM_OFFSET, size){}
};
class EWRAM : public MemoryRegion
{
public:
	EWRAM(uint16_t size) : MemoryRegion(EWRAM_OFFSET, size){}
};
class WRAM : public MemoryRegion
{
public:
	WRAM(uint16_t size) : MemoryRegion(WRAM_OFFSET, size){}
};
class OAM : public MemoryRegion
{
public:
	OAM(uint16_t size) : MemoryRegion(OAM_OFFSET, size){}
};
class MMIO : public MemoryRegion
{
public:
	MMIO(uint16_t size) : MemoryRegion(MMIO_OFFSET, size){}
};
class HighRAM : public MemoryRegion
{
public:
	HighRAM(uint16_t size) : MemoryRegion(HIGH_RAM_OFFSET, size){}
};
