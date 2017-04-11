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

#include "MemoryController.h"

#include <memory>


namespace Core {
    class GameBoy;
} // namespace Core

namespace Memory {

class SystemMemoryController
: public MemoryController
{
    // System memory pages
    std::unique_ptr<MemoryPage> PageROM;
    std::unique_ptr<MemoryPage> PageVRAM;
    std::unique_ptr<MemoryPage> PageSRAM;
    std::unique_ptr<MemoryPage> PageWRAM;
    std::unique_ptr<MemoryPage> PageOAM;
    std::unique_ptr<MemoryPage> PageHighRAM;
    std::unique_ptr<MemoryPage>& GetPage(u16 address);

public:
    SystemMemoryController(Core::GameBoy* gameboy);

    virtual void Write8(u16 address, u8 data);
    virtual void Write16(u16 address, u16 data);
    virtual u8 Read8(u16 address);
    virtual u16 Read16(u16 address);

    virtual void WriteBytes(const u8* src, u16 destination, u16 size);
    virtual void ReadBytes(u8* destination, u16 src, u16 size);
};

}; // namespace Memory
