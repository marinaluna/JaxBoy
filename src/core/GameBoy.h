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

#pragma once

#include "../common/Types.h"

#include <memory>
#include <vector>


namespace Debug {
    class Logger;
}; // namespace Debug

namespace Memory {
    class MemoryBus;
    class IORegisterMemoryController;
}; // namespace Memory

namespace Core {
class Processor;
class PPU;
class Rom;

class GameBoy
{
public:
    struct Options
    {
        bool isDebug = false;
        int scale = 1;
    };

    GameBoy(GameBoy::Options& options,
            const std::vector<u8>& rom,
            const std::vector<u8>& bootrom);

    void Run();
    void Stop()
        { Stopped = true; }

    void SystemError(const std::string& error_msg);

    bool IsInBootROM()
        { return InBootROM; }
    std::unique_ptr<Rom>& GetCurrentROM()
        { return game_rom; };

private:
    friend class Memory::IORegisterMemoryController;

    // Options configuration
    GameBoy::Options _Options;

    // Components
    std::unique_ptr<Processor> processor;
    std::unique_ptr<PPU> ppu;
    std::unique_ptr<Rom> game_rom;
    // System memory map
    std::shared_ptr<Memory::MemoryBus> memory_bus;
    // Logger
    std::shared_ptr<Debug::Logger> logger;

    bool InBootROM = false;
    bool Stopped = false;
};

}; // namespace Core
