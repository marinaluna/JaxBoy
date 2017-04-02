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

#include "../../common/Types.h"

#include <vector>


struct MemoryPage
{
    std::vector<u8> bytes;
    u16 base;
    MemoryPage(u16 size, u16 base): bytes(size), base(base){}
};

namespace Core {

class GameBoy;

class MemoryMap
{
    GameBoy* gameboy;

    // 0x0000-0x7FFF
    MemoryPage RegionROM;
    // 0x8000-0x9FFF
    MemoryPage RegionVRAM;
    // 0xA000-0xBFFF
    MemoryPage RegionSaveRAM;
    // 0xC000-0xDFFF
    MemoryPage RegionWRAM;
    // 0xFE00-0xFE9F
    MemoryPage RegionOAM;
    // 0xFF80-0xFFFE
    MemoryPage RegionZeroPage;

    MemoryPage* GetPage(u16 address);

public:

    MemoryMap(GameBoy* gameboy);

    void Write8(u16 address, u8 data);
    void Write16(u16 address, u16 data);
    u8 Read8(u16 address);
    u16 Read16(u16 address);

    void WriteBytes(u16 address, const std::vector<u8>& src, u16 startOffset, u16 bytes);
    void CopyBytes(u8* destination, u16 address, u16 bytes);
};

}; // namespace Core
