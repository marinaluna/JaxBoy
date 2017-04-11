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

#include "IORegisterMemoryController.h"
#include "../../GameBoy.h"
#include "../../Rom.h"
#include "../../PPU.h"
#include "../../processor/Processor.h"
#include "../../memory/MemoryBus.h"

#include "../../../common/Globals.h"


namespace Memory {

IORegisterMemoryController::IORegisterMemoryController(Core::GameBoy* gameboy)
:
    MemoryController (gameboy)
{}

void IORegisterMemoryController::Write8(u16 address, u8 data)
{
    switch(address)
    {
        case 0xFF0F:
            // interrupt request flags
            gameboy->processor->InterruptsRequested = data;
            break;
        case 0xFF40:
            gameboy->ppu->LCDC = data;
            break;
        case 0xFF41:
            gameboy->ppu->STAT = data;
            break;
        case 0xFF42:
            gameboy->ppu->ScrollY = data;
            break;
        case 0xFF43:
            gameboy->ppu->ScrollX = data;
            break;
        case 0xFF44:
            // Writing to this resets it
            gameboy->ppu->Line = 0;
            break;
        case 0xFF45:
            gameboy->ppu->LineCompare = data;
            break;
        case 0xFF46:
            gameboy->processor->StartDMATransfer(data);
            break;
        case 0xFF47:
            gameboy->ppu->BGPalette[0] = gColors[(data & 0b00000011) >> 0];
            gameboy->ppu->BGPalette[1] = gColors[(data & 0b00001100) >> 2];
            gameboy->ppu->BGPalette[2] = gColors[(data & 0b00110000) >> 4];
            gameboy->ppu->BGPalette[3] = gColors[(data & 0b11000000) >> 6];
            break;
        case 0xFF48:
            gameboy->ppu->OBJ0Palette[0] = gColors[(data & 0b00000011) >> 0];
            gameboy->ppu->OBJ0Palette[1] = gColors[(data & 0b00001100) >> 2];
            gameboy->ppu->OBJ0Palette[2] = gColors[(data & 0b00110000) >> 4];
            gameboy->ppu->OBJ0Palette[3] = gColors[(data & 0b11000000) >> 6];
            break;
        case 0xFF49:
            gameboy->ppu->OBJ1Palette[0] = gColors[(data & 0b00000011) >> 0];
            gameboy->ppu->OBJ1Palette[1] = gColors[(data & 0b00001100) >> 2];
            gameboy->ppu->OBJ1Palette[2] = gColors[(data & 0b00110000) >> 4];
            gameboy->ppu->OBJ1Palette[3] = gColors[(data & 0b11000000) >> 6];
            break;
        case 0xFF50:
            // replace ROM interrupt vectors
            gameboy->memory_bus->WriteBytes(gameboy->game_rom->GetBytes().data(), 0x0000, 0x100);
            gameboy->InBootROM = false;
            break;
        case 0xFFFF:
            // interrupt enable flags
            gameboy->processor->InterruptsEnabled = data;
            break;
    }
}

u8 IORegisterMemoryController::Read8(u16 address)
{
    switch(address)
    {
        case 0xFF0F:
            return gameboy->processor->InterruptsRequested;
        case 0xFF40:
            return gameboy->ppu->LCDC;
        case 0xFF41:
            return gameboy->ppu->STAT;
        case 0xFF42:
            return gameboy->ppu->ScrollY;
        case 0xFF43:
            return gameboy->ppu->ScrollX;
        case 0xFF44:
            return gameboy->ppu->Line;
        case 0xFF45:
            return gameboy->ppu->LineCompare;
        case 0xFFFF:
            return gameboy->processor->InterruptsEnabled;
    }
    // GameBoy system bus returns 0xFF by default
    return 0xFF;
}

}; // namespace Memory
