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

#include "MBC.h"

#include "../../GameBoy.h"

#include <string>


namespace Memory {

MBC::MBC(Core::GameBoy* gameboy)
:   gameboy(gameboy),
    romBank0(new MemoryPage(0x0000, 0x4000)),
    romBank1(new MemoryPage(0x4000, 0x4000)),
    vram(new MemoryPage(0x8000, 0x2000)),
    sram(new MemoryPage(0xA000, 0x2000)),
    wram(new MemoryPage(0xC000, 0x2000)),
    oam(new MemoryPage(0xFE00, 0x00A0)),
    highRam(new MemoryPage(0xFF80, 0x007F))
{}

std::unique_ptr<MemoryPage>& MBC::GetPage(u16 address)
{
    if(address >= 0x0000 && address <= 0x3FFF)
        return romBank0;
    if(address >= 0x4000 && address <= 0x7FFF)
        return romBank1;
    if(address >= 0x8000 && address <= 0x9FFF)
        return vram;
    if(address >= 0xA000 && address <= 0xBFFF)
        return sram;
    if(address >= 0xC000 && address <= 0xDFFF)
        return wram;
    if(address >= 0xFE00 && address <= 0xFE9F)
        return oam;
    if(address >= 0xFF80 && address <= 0xFFFE)
        return highRam;

    throw std::out_of_range("Address out of bounds!");
}

void MBC::Write8(u16 address, u8 data)
{
    try
    {
        std::unique_ptr<MemoryPage>& page = GetPage(address);
        address -= page->GetBase();
        page->GetBytes().at(address) = data;
    }
    catch(std::out_of_range& e)
    {
        gameboy->SystemError("Memory write out of range!");
    }
}

void MBC::Write16(u16 address, u16 data)
{
    try
    {
        std::unique_ptr<MemoryPage>& page = GetPage(address);
        address -= page->GetBase();
        page->GetBytes().at(address) = data & 0x00FF;
        page->GetBytes().at(address + 1) = (data & 0xFF00) >> 8;
    }
    catch(std::out_of_range& e)
    {
        gameboy->SystemError("Memory write out of range!");
    }
}

u8 MBC::Read8(u16 address)
{
    try
    {
        std::unique_ptr<MemoryPage>& page = GetPage(address);
        address -= page->GetBase();
        return page->GetBytes().at(address);
    }
    catch(std::out_of_range& e)
    {
        gameboy->SystemError("Memory read out of range!");
        return 0xFF;
    }
}

u16 MBC::Read16(u16 address)
{
    try
    {
        std::unique_ptr<MemoryPage>& page = GetPage(address);
        address -= page->GetBase();
        return page->GetBytes().at(address) | (page->GetBytes().at(address + 1) << 8);
    }
    catch(std::out_of_range& e)
    {
        gameboy->SystemError("Memory read out of range!");
        return 0xFFFF;
    }
}

void MBC::WriteBytes(const u8* src, u16 destination, u16 size)
{
    try
    {
        // TODO: won't work across page boundaries
        std::unique_ptr<MemoryPage>& page = GetPage(destination);
        std::memcpy(page->GetRaw() + (destination - page->GetBase()), src, size);
    }
    catch(std::out_of_range& e)
    {
        gameboy->SystemError("Error writing bytes!");
    }
}

void MBC::ReadBytes(u8* destination, u16 src, u16 size)
{
    try
    {
        std::unique_ptr<MemoryPage>& page = GetPage(src);
        std::memcpy(destination, page->GetRaw() + (src - page->GetBase()), size);
    }
    catch(std::out_of_range& e)
    {
        gameboy->SystemError("Error reading bytes!");
    }
}

}; // namespace Memory