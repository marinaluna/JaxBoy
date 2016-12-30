#include "MemoryRegion.h"

#include "../GameBoy.h"
#include "../constants.h"

#include "MemoryMap.h"


MemoryMap::MemoryMap(size_t size)
{
	program_rom_area = new PGROM(0x8000);
	video_ram_area = new VRAM(0x2000);
	external_ram_area = new EWRAM(0x2000);
	internal_ram_area = new WRAM(0x2000);
	oam_area = new OAM(0xA0);
	mmio_area = new MMIO(0x80);
	high_ram_area = new HighRAM(0x7F);
}

MemoryRegion* MemoryMap::GetRegionFromAddress(uint16_t address)
{
	if(address >= PGROM_OFFSET)
	{
		if(address >= VRAM_OFFSET)
		{
			if(address >= EWRAM_OFFSET)
			{
				if(address >= WRAM_OFFSET)
				{
					if(address >= FORBIDDEN1_AREA_OFFSET)
					{
						if(address >= OAM_OFFSET)
						{
							if(address >= FORBIDDEN2_AREA_OFFSET)
							{
								if(address >= MMIO_OFFSET)
								{
									if(address >= HIGH_RAM_OFFSET)
									{
										return high_ram_area;
									}
									return mmio_area;
								}
								return NULL;
							}
							return oam_area;
						}
						return NULL;
					}
					return internal_ram_area;
				}
				return external_ram_area;
			}
			return video_ram_area;
		}
		return program_rom_area;
	}
	return NULL;
}

// All reads and writes are in little endian
void MemoryMap::write8(uint16_t address, uint8_t data)
{
	MemoryRegion* region = GetRegionFromAddress(address);
	if(region == NULL)
	{
		GameBoy::SystemError("Attemped to access forbidden memory at address %#04x\n", address);
	}
	region->write8(address, data);
}
void MemoryMap::write16(uint16_t address, uint16_t data)
{
	MemoryRegion* region = GetRegionFromAddress(address);
	if(region == NULL)
	{
		GameBoy::SystemError("Attemped to access forbidden memory at address %#04x\n", address);
	}
	region->write16(address, data);
}
uint8_t MemoryMap::read8(uint16_t address)
{
	MemoryRegion* region = GetRegionFromAddress(address);
	if(region == NULL)
	{
		GameBoy::SystemError("Attemped to access forbidden memory at address %#04x\n", address);
	}
	return region->read8(address);
}
uint16_t MemoryMap::read16(uint16_t address)
{
	MemoryRegion* region = GetRegionFromAddress(address);
	if(region == NULL)
	{
		GameBoy::SystemError("Attemped to access forbidden memory at address %#04x\n", address);
	}
	return region->read16(address);
}
