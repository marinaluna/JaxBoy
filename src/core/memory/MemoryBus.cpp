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

#include "MemoryBus.h"

#include "mbc/MBC.h"
#include "mbc/MBC1.h"
#include "mbc/MBC3.h"

#include "../GameBoy.h"
#include "../PPU.h"
#include "../Rom.h"
#include "../processor/Processor.h"

#include "../../common/Globals.h"

#include <string>


static bool CheckBounds8(u16 address)
{
    if((address >= 0xE000 && address <= 0xFDFF) ||
       (address >= 0xFEA0 && address <= 0xFEFF))
        return false;

    return true;
}
static bool CheckBounds16(u16 address)
{
    if((address >= 0xE000 && address <= 0xFDFF) ||
       (address+1 >= 0xE000 && address+1 <= 0xFDFF) ||
       (address >= 0xFEA0 && address <= 0xFEFF) ||
       (address+1 >= 0xFEA0 && address+1 <= 0xFEFF))
        return false;

    return true;
}


namespace Memory {

bool MemoryBus::TryIOWrite(u16 address, u8 data)
{
    if((address >= 0xFF00 && address < 0xFF80) || address == 0xFFFF)
    {
        switch(address & 0x00FF)
        {
        case 0x00:
            // Controller input
            // 0x30 means no controller polling
            gameboy->P1 = (gameboy->P1 & 0x0F) | (data & 0x30);
            break;
        case 0x0F:
            // interrupt request flags
            gameboy->processor->IF = data;
            break;
        case 0x40:
            gameboy->ppu->LCDC = data;
            break;
        case 0x41:
            gameboy->ppu->STAT = data;
            break;
        case 0x42:
            gameboy->ppu->SCY = data;
            break;
        case 0x43:
            gameboy->ppu->SCX = data;
            break;
        case 0x44:
            // Writing to this resets it
            gameboy->ppu->LY = 0;
            break;
        case 0x45:
            gameboy->ppu->LYC = data;
            break;
        case 0x46:
            gameboy->processor->StartDMATransfer(data);
            break;
        case 0x47:
            gameboy->ppu->BGPalette[0] = gColors[(data & 0b00000011) >> 0];
            gameboy->ppu->BGPalette[1] = gColors[(data & 0b00001100) >> 2];
            gameboy->ppu->BGPalette[2] = gColors[(data & 0b00110000) >> 4];
            gameboy->ppu->BGPalette[3] = gColors[(data & 0b11000000) >> 6];
            break;
        case 0x48:
            gameboy->ppu->OBJ0Palette[0] = gColors[(data & 0b00000011) >> 0];
            gameboy->ppu->OBJ0Palette[1] = gColors[(data & 0b00001100) >> 2];
            gameboy->ppu->OBJ0Palette[2] = gColors[(data & 0b00110000) >> 4];
            gameboy->ppu->OBJ0Palette[3] = gColors[(data & 0b11000000) >> 6];
            break;
        case 0x49:
            gameboy->ppu->OBJ1Palette[0] = gColors[(data & 0b00000011) >> 0];
            gameboy->ppu->OBJ1Palette[1] = gColors[(data & 0b00001100) >> 2];
            gameboy->ppu->OBJ1Palette[2] = gColors[(data & 0b00110000) >> 4];
            gameboy->ppu->OBJ1Palette[3] = gColors[(data & 0b11000000) >> 6];
            break;
        case 0x4A:
            gameboy->ppu->WY = data;
            break;
        case 0x4B:
            gameboy->ppu->WX = data;
            break;
        case 0x50:
            // replace ROM interrupt vectors
            gameboy->memory_bus->WriteBytes(gameboy->game_rom->GetBytes().data(), 0x0000, 0x100);
            gameboy->InBootROM = false;
            break;
        case 0xFF:
            // interrupt enable flags
            gameboy->processor->IE = data;
            break;
        }

        return true;
    }

    return false;
}

bool MemoryBus::TryIORead(u16 address, u8& retval)
{
    if((address >= 0xFF00 && address < 0xFF80) || address == 0xFFFF)
    {
        switch(address & 0x00FF)
        {
        case 0x00:
            retval = gameboy->P1 | 0xC0; break;
        case 0x0F:
            retval = gameboy->processor->IF; break;
        case 0x40:
            retval = gameboy->ppu->LCDC; break;
        case 0x41:
            retval = gameboy->ppu->STAT; break;
        case 0x42:
            retval = gameboy->ppu->SCY; break;
        case 0x43:
            retval = gameboy->ppu->SCX; break;
        case 0x44:
            retval = gameboy->ppu->LY; break;
        case 0x45:
            retval = gameboy->ppu->LYC; break;
        case 0x4A:
            retval = gameboy->ppu->WY; break;
        case 0x4B:
            retval = gameboy->ppu->WX; break;
        case 0xFF:
            retval = gameboy->processor->IE; break;
        }

        return true;
    }

    return false;
}

void MemoryBus::InitMBC(std::unique_ptr<Core::Rom>& rom)
{
    switch(rom->GetCartType())
    {
    case 0x00: // Cart Only
        mbc = std::unique_ptr<MBC> (new MBC(gameboy)); break;
    case 0x01: // MBC1
        mbc = std::unique_ptr<MBC> (new MBC1(gameboy)); break;
    case 0x13: // MBC3 + RAM + BATTERY
        mbc = std::unique_ptr<MBC> (new MBC3(gameboy)); break;
    default:
        throw std::runtime_error("MBC type unknown! " + std::to_string(rom->GetCartType()));
    }

    mbc->Load(rom);
}

void MemoryBus::Write8(u16 address, u8 data)
{
    if(!CheckBounds8(address))
        return;
    if(TryIOWrite(address, data))
        return;

    mbc->Write8(address, data);
}

void MemoryBus::Write16(u16 address, u16 data)
{
    if(!CheckBounds16(address))
        return;

    mbc->Write16(address, data);
}

u8 MemoryBus::Read8(u16 address)
{
    if(!CheckBounds8(address))
        return 0xFF;
    u8 data;
    if(TryIORead(address, data))
        return data;

    return mbc->Read8(address);
}

u16 MemoryBus::Read16(u16 address)
{
    if(!CheckBounds16(address))
        return 0xFFFF;

    return mbc->Read16(address);
}
// Use raw buffers for these rather than vectors
// because man is std::vector slow...
void MemoryBus::WriteBytes(const u8* src, u16 destination, u16 size)
{
    mbc->WriteBytes(src, destination, size);
}

void MemoryBus::ReadBytes(u8* destination, u16 src, u16 size)
{
    mbc->ReadBytes(destination, src, size);
}

}; // namespace Memory
