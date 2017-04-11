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
#include "../GameBoy.h"
#include "../Rom.h"

#include "controller/SystemMemoryController.h"
#include "controller/IORegisterMemoryController.h"


namespace Memory {

MemoryBus::MemoryBus(Core::GameBoy* gameboy)
:
    gameboy (gameboy),

    // Initialize system memory controllers
    defaultMemoryController (new SystemMemoryController(gameboy)),
    ioMemoryController (new IORegisterMemoryController(gameboy))
{}

std::unique_ptr<MemoryController>& MemoryBus::GetMemoryController(u16 address)
{
    // if this is ROM, use the ROM's MBC
    if(address >= 0x0000 && address < 0x8000)
    {
        return defaultMemoryController;// gameboy->GetCurrentROM()->GetMBC();
    }
    // if this is in IO register range, return a custom controller
    if((address >= 0xFF00 && address < 0xFF80) || address == 0xFFFF)
    {
        return ioMemoryController;
    }
    // Else, use the default system memory controller
    return defaultMemoryController;
}

void MemoryBus::Write8(u16 address, u8 data)
{
    // These are forbidden areas
    // Due to a bug, Tetris writes here, so
    // handle gracefully rather than aborting
    if((address >= 0xE000 && address <= 0xFDFF) ||
       (address >= 0xFEA0 && address <= 0xFEFF))
        return;

    GetMemoryController(address)->Write8(address, data);
}

void MemoryBus::Write16(u16 address, u16 data)
{
    if((address >= 0xE000 && address <= 0xFDFF) ||
       (address+1 >= 0xE000 && address+1 <= 0xFDFF) ||
       (address >= 0xFEA0 && address <= 0xFEFF) ||
       (address+1 >= 0xFEA0 && address+1 <= 0xFEFF))
        return;

    GetMemoryController(address)->Write16(address, data);
}

u8 MemoryBus::Read8(u16 address)
{
    if((address >= 0xE000 && address <= 0xFDFF) ||
       (address >= 0xFEA0 && address <= 0xFEFF))
        return 0xFF;

    return GetMemoryController(address)->Read8(address);
}

u16 MemoryBus::Read16(u16 address)
{
    if((address >= 0xE000 && address <= 0xFDFF) ||
       (address+1 >= 0xE000 && address+1 <= 0xFDFF) ||
       (address >= 0xFEA0 && address <= 0xFEFF) ||
       (address+1 >= 0xFEA0 && address+1 <= 0xFEFF))
        return 0xFFFF;

    return GetMemoryController(address)->Read16(address);
}
// Use raw buffers for these rather than vectors
// because man is std::vector slow...
void MemoryBus::WriteBytes(const u8* src, u16 destination, u16 size)
{
    GetMemoryController(destination)->WriteBytes(src, destination, size);
}

void MemoryBus::ReadBytes(u8* destination, u16 src, u16 size)
{
    GetMemoryController(src)->ReadBytes(destination, src, size);
}

}; // namespace Memory
