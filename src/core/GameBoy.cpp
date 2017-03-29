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

#include "GameBoy.h"
#include "PPU.h"
#include "Rom.h"
#include "memory/MemoryMap.h"
#include "processor/Processor.h"

#include "../common/Globals.h"

#include "../debugger/Logger.h"

#include <memory>
#include <vector>


namespace Core {

GameBoy::GameBoy(GameBoy::Options& options, const std::vector<u8>& rom, const std::vector<u8>& bootrom)
:
   _Options (options)
{
    memory_map = std::make_shared<MemoryMap>(this);

    logger = std::make_shared<Debugger::Logger>(memory_map);
    processor = std::unique_ptr<Processor> (new Processor(this, memory_map, logger));
    ppu = std::unique_ptr<PPU> (new PPU(this, 160+2, 144+25, memory_map, logger));

    game_rom = std::unique_ptr<Rom> (new Rom(rom));

    // load boot ROM at 0x0000
    memory_map->WriteBytes(0x0000, bootrom, 0x0000, 0x100);
    // load ROM at 0x0000 + 0x100 bytes
    // the rest is loaded after boot ROM finishes
    memory_map->WriteBytes(0x0100, rom, 0x0100, 0x8000 - 0x100);

    InBootROM = true;
}

void GameBoy::Run()
{
    while(!Stopped)
    {
        int cycles = processor->Tick();
        // if Escape was pressed or the Window was closed
        if(ppu->Tick(cycles) == -1)
        {
            Stop();
        }
    }
}

void GameBoy::Stop()
{
    Stopped = true;
}

void GameBoy::IORegisterWrite(u16 address, u8 data)
{
    switch(address)
    {
        case 0xFF40:
            ppu->LCDC = data;
            break;
        case 0xFF42:
            ppu->ScrollY = data;
            break;
        case 0xFF43:
            ppu->ScrollX = data;
            break;
        case 0xFF44:
            // Writing to this resets it
            ppu->Line = 0;
            break;
        case 0xFF45:
            ppu->LineCompare = data;
            break;
        case 0xFF47:
            ppu->BGPalette[0] = gColors[(data & 0b00000011) >> 0];
            ppu->BGPalette[1] = gColors[(data & 0b00001100) >> 2];
            ppu->BGPalette[2] = gColors[(data & 0b00110000) >> 4];
            ppu->BGPalette[3] = gColors[(data & 0b11000000) >> 6];
            break;
        case 0xFF50:
            // replace ROM interrupt vectors
            memory_map->WriteBytes(0x0000, game_rom->bytes, 0x0000, 0x100);
            InBootROM = false;
            break;
    }
}

u8 GameBoy::IORegisterRead(u16 address)
{
    switch(address)
    {
        case 0xFF40:
            return ppu->LCDC;
        case 0xFF42:
            return ppu->ScrollY;
        case 0xFF43:
            return ppu->ScrollX;
        case 0xFF44:
            return ppu->Line;
        case 0xFF45:
            return ppu->LineCompare;
    }
    // GameBoy system bus returns 0xFF by default
    return 0xFF;
}

}; // namespace Core