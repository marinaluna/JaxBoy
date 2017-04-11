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
// I have no idea why I need to include this
#include "controller/MemoryController.h"

#include "../../common/Types.h"

#include <memory>


namespace Core {
    class GameBoy;
}; // namespace Core

namespace Memory {
class MemoryController;

class MemoryBus
{
    // Default controllers for system memory
    // and for handling IO Register reads/writes
    std::unique_ptr<MemoryController> defaultMemoryController;
    std::unique_ptr<MemoryController> ioMemoryController;
    std::unique_ptr<MemoryController>& GetMemoryController(u16 address);

    Core::GameBoy* gameboy;
public:
    MemoryBus(Core::GameBoy* gameboy);

    void Write8(u16 address, u8 data);
    void Write16(u16 address, u16 data);
    u8 Read8(u16 address);
    u16 Read16(u16 address);

    void WriteBytes(const u8* src, u16 destination, u16 size);
    void ReadBytes(u8* destination, u16 src, u16 size);
};

}; // namespace Memory
