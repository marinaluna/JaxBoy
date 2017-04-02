// Copyright (C) 2017 Ryan Terry
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "MemoryMap.h"

#include "../GameBoy.h"

#include <vector>
#include <cstring>


namespace Core {

MemoryMap::MemoryMap(GameBoy* gameboy)
:
    gameboy(gameboy),

    RegionROM      (0x8000, 0x0000),
    RegionVRAM     (0x2000, 0x8000),
    RegionSaveRAM  (0x2000, 0xA000),
    RegionWRAM     (0x2000, 0xC000),
    RegionOAM      (0x00A0, 0xFE00),
    RegionZeroPage (0x007F, 0xFF80)
{}

MemoryPage* MemoryMap::GetPage(u16 address)
{
    if(address >= 0x0000 && address <= 0x7FFF)
        return &RegionROM;
    if(address >= 0x8000 && address <= 0x9FFF)
        return &RegionVRAM;
    if(address >= 0xA000 && address <= 0xBFFF)
        return &RegionSaveRAM;
    if(address >= 0xC000 && address <= 0xDFFF)
        return &RegionWRAM;
    if(address >= 0xFE00 && address <= 0xFE9F)
        return &RegionOAM;
    if(address >= 0xFF80 && address <= 0xFFFE)
        return &RegionZeroPage;
    // If no page can be found
    return nullptr;
}

void MemoryMap::Write8(u16 address, u8 data)
{
    if((address >= 0xFF00 && address < 0xFF80) || address == 0xFFFF)
    {
        gameboy->IORegisterWrite(address, data);
    }
    else
    {
        MemoryPage* region = GetPage(address);
        if(region != nullptr)
        {
            region->bytes.at(address - region->base) = data;
        }
    }
}

void MemoryMap::Write16(u16 address, u16 data)
{
    MemoryPage* region = GetPage(address);
    if(region != nullptr)
    {
        region->bytes.at(address - region->base) = data & 0xFF;
        region->bytes.at(address - region->base + 1) = (data & 0xFF00) >> 8;
    }
}

u8 MemoryMap::Read8(u16 address)
{
    if((address >= 0xFF00 && address < 0xFF80) || address == 0xFFFF)
    {
        return gameboy->IORegisterRead(address);
    }
    MemoryPage* region = GetPage(address);
    if(region != nullptr)
    {
        return region->bytes.at(address - region->base);
    }
    return 0xFF;
}

u16 MemoryMap::Read16(u16 address)
{
    MemoryPage* region = GetPage(address);
    if(region != nullptr)
    {
        return (region->bytes.at(address - region->base)) | (region->bytes.at(address - region->base + 1) << 8);
    }
    return 0xFF;
}

void MemoryMap::WriteBytes(u16 address, const std::vector<u8>& data, u16 startOffset, u16 bytes)
{
    for(int i = 0; i < bytes; i++)
    {
        MemoryPage* region = GetPage(address);
        if(region != nullptr)
        {
            region->bytes.at(address - region->base + i) = data.at(i + startOffset);
        }
    }
}

void MemoryMap::CopyBytes(u8* destination, u16 address, u16 bytes)
{
    MemoryPage* region = GetPage(address);
    if(region != nullptr)
    {
        std::memcpy(destination, region->bytes.data() + (address - region->base), bytes);
    }
}

}; // namespace Core
