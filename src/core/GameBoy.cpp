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
#include "processor/Processor.h"
#include "memory/MemoryBus.h"

#include "../common/Globals.h"

#include "../debug/Logger.h"

#include <string>


namespace Core {

GameBoy::GameBoy(GameBoy::Options& options,
                 const std::vector<u8>& rom,
                 const std::vector<u8>& bootrom)
:
    _Options (options)
{
    memory_bus = std::make_shared<Memory::MemoryBus>(this);

    processor = std::unique_ptr<Processor> (new Processor(this, memory_bus));
    ppu = std::unique_ptr<PPU> (new PPU(this, 160, 144, 1, memory_bus));

    game_rom = std::unique_ptr<Rom> (new Rom(rom));
    // load ROM at 0x0000-0x7FFF
    memory_bus->InitMBC(game_rom);

    // load boot ROM at 0x0000-0x00FF
    memory_bus->WriteBytes(bootrom.data(), 0x0000, 0x0100);

    InBootROM = true;
}

void GameBoy::Cycle()
{
    int cycles = processor->Tick();
        
    if(ppu->Update(cycles) == -1)
    {
        Stop();
    }
}

void GameBoy::SystemError(const std::string& error_msg)
{
    LOG_ERROR(error_msg);
    Stop();
}

}; // namespace Core
