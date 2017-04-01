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


namespace Core {

MemoryMap::MemoryMap(GameBoy* gameboy)
:
    gameboy(gameboy),

    RegionROM (0x8000),
    RegionVRAM (0x2000),
    RegionSaveRAM (0x2000),
    RegionWRAM (0x2000),
    RegionOAM (0x00A0),
    RegionZeroPage (0x007F)
{}

MemoryRegion* MemoryMap::GetRegion(u16 address, u16& pagestart)
{
    if(address >= 0x0000 && address <= 0x7FFF)
    {
        pagestart = 0x0000;
        return &RegionROM;
    }
    if(address >= 0x8000 && address <= 0x9FFF)
    {
        pagestart = 0x8000;
        return &RegionVRAM;
    }
    if(address >= 0xA000 && address <= 0xBFFF)
    {
        pagestart = 0xA000;
        return &RegionSaveRAM;
    }
    if(address >= 0xC000 && address <= 0xDFFF)
    {
        pagestart = 0xC000;
        return &RegionWRAM;
    }
    if(address >= 0xFE00 && address <= 0xFE9F)
    {
        pagestart = 0xFE00;
        return &RegionOAM;
    }
    if(address >= 0xFF80 && address <= 0xFFFE)
    {
        pagestart = 0xFF80;
        return &RegionZeroPage;
    }

    // If no region can be found
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
        u16 pagestart;
        MemoryRegion* region = GetRegion(address, pagestart);
        if(region != nullptr)
        {
            region->at(address - pagestart) = data;
        }
    }
}

void MemoryMap::Write16(u16 address, u16 data)
{
    u16 pagestart;
    MemoryRegion* region = GetRegion(address, pagestart);
    if(region != nullptr)
    {
        region->at(address - pagestart) = data & 0xFF;
        region->at(address - pagestart + 1) = (data & 0xFF00) >> 8;
    }
}

u8 MemoryMap::Read8(u16 address)
{
    if((address >= 0xFF00 && address < 0xFF80) || address == 0xFFFF)
    {
        return gameboy->IORegisterRead(address);
    }
    u16 pagestart;
    MemoryRegion* region = GetRegion(address, pagestart);
    if(region != nullptr)
    {
        return region->at(address - pagestart);
    }
    return 0xFF;
}

u16 MemoryMap::Read16(u16 address)
{
    u16 pagestart;
    MemoryRegion* region = GetRegion(address, pagestart);
    if(region != nullptr)
    {
        return (region->at(address - pagestart)) | (region->at(address - pagestart + 1) << 8);
    }
    return 0xFF;
}

void MemoryMap::WriteBytes(u16 address, const std::vector<u8>& data, u16 startOffset, u16 bytes)
{
    for(int i = 0; i < bytes; i++)
    {
        u16 pagestart;
        MemoryRegion* region = GetRegion(address, pagestart);
        if(region != nullptr)
        {
            region->at(address - pagestart + i) = data.at(i + startOffset);
        }
    }
}

}; // namespace Core
