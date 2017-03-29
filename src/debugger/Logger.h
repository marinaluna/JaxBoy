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


namespace Core {
    class MemoryMap;
    class Processor;
}; // namespace Core

namespace Debugger {

class Logger
{
    std::shared_ptr<Core::MemoryMap> memory_map;

public:
    Logger(std::shared_ptr<Core::MemoryMap>& memory_map);

    void LogRegisters(const Core::Processor& processor);
    void LogIORegisters();
    void LogMemory(u16 address, u16 bytes);
    void LogVRAM();
    void LogDisassembly(u16 address, u16 bytes);
};

}; // namespace Debugger
