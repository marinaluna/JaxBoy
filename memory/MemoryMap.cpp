#include "../GameBoy.h"
#include "../PPU.h"
#include "../util/Macros.h"

#include "MemoryRegion.h"

#include "MemoryMap.h"


MemoryMap::MemoryMap()
{
    // setup memory pages
    program_rom_region = new MemoryRegion(PGROM_REGION, 0x8000);
    video_ram_region = new MemoryRegion(VRAM_REGION, 0x2000);
    external_ram_region = new MemoryRegion(EWRAM_REGION, 0x2000);
    internal_ram_region = new MemoryRegion(WRAM_REGION, 0x2000);
    forbidden1_region = new ForbiddenRegion(FORBIDDEN1_REGION, 0x1E00);
    oam_region = new MemoryRegion(OAM_REGION, 0xA0);
    forbidden2_region = new ForbiddenRegion(FORBIDDEN2_REGION, 0x60);
    mmio_region = new MemoryRegion(MMIO_REGION, 0x80);
    high_ram_region = new MemoryRegion(HIGH_RAM_REGION, 0x7F);
}

MemoryMap::~MemoryMap()
{
    if(program_rom_region != nullptr)
        delete program_rom_region;
    if(video_ram_region != nullptr)
        delete video_ram_region;
    if(external_ram_region != nullptr)
        delete external_ram_region;
    if(internal_ram_region != nullptr)
        delete internal_ram_region;
    if(forbidden1_region != nullptr)
        delete forbidden1_region;
    if(oam_region != nullptr)
        delete oam_region;
    if(forbidden2_region != nullptr)
        delete forbidden2_region;
    if(mmio_region != nullptr)
        delete mmio_region;
    if(high_ram_region != nullptr)
        delete high_ram_region;
}

MemoryRegion* MemoryMap::GetRegionFromAddress(uint16_t address)
{
    if(address >= PGROM_REGION)
    {
        if(address >= VRAM_REGION)
        {
            if(address >= EWRAM_REGION)
            {
                if(address >= WRAM_REGION)
                {
                    if(address >= FORBIDDEN1_REGION)
                    {
                        if(address >= OAM_REGION)
                        {
                            if(address >= FORBIDDEN2_REGION)
                            {
                                if(address >= MMIO_REGION)
                                {
                                    if(address >= HIGH_RAM_REGION)
                                    {
                                        return high_ram_region;
                                    }
                                    return mmio_region;
                                }
                                return forbidden2_region;
                            }
                            return oam_region;
                        }
                        return forbidden1_region;
                    }
                    return internal_ram_region;
                }
                return external_ram_region;
            }
            return video_ram_region;
        }
        return program_rom_region;
    }
    return NULL;
}

void MemoryMap::IOSpecialWrite(uint16_t address, uint8_t data)
{
    // PPU registers; ignore DMA though
    if(address != 0xFF46 && address >= 0xFF40 && address <= 0xFF4B)
    {
        GameBoy::GetPPU().WriteRegister(address, data);
    }
}

bool MemoryMap::IOSpecialRead(uint16_t address, uint8_t& retval)
{
    if(address != 0xFF46 && address >= 0xFF40 && address <= 0xFF4B)
    {
        retval = GameBoy::GetPPU().ReadRegister(address);
        return true;
    }

    return false;
}

// All reads and writes are in little endian
void MemoryMap::Write8(uint16_t address, uint8_t data)
{
    IOSpecialWrite(address, data);

    MemoryRegion* region = GetRegionFromAddress(address);
    if(region == NULL)
    {
        GameBoy::SystemError("Attemped to access forbidden memory at address %#04x\n", address);
    }
    region->write8(address, data);
}
void MemoryMap::Write16(uint16_t address, uint16_t data)
{
    MemoryRegion* region = GetRegionFromAddress(address);
    if(region == NULL)
    {
        GameBoy::SystemError("Attemped to access forbidden memory at address %#04x\n", address);
    }
    region->write16(address, data);
}
uint8_t MemoryMap::Read8(uint16_t address)
{
    uint8_t specialret;
    if(IOSpecialRead(address, specialret))
    {
        return specialret;
    }

    MemoryRegion* region = GetRegionFromAddress(address);
    if(region == NULL)
    {
        GameBoy::SystemError("Attemped to access forbidden memory at address %#04x\n", address);
    }
    return region->read8(address);
}
uint16_t MemoryMap::Read16(uint16_t address)
{
    MemoryRegion* region = GetRegionFromAddress(address);
    if(region == NULL)
    {
        GameBoy::SystemError("Attemped to access forbidden memory at address %#04x\n", address);
    }
    return region->read16(address);
}
