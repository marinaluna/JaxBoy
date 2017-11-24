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
#include "../../../common/Types.h"

#include <memory>
#include <vector>


namespace Core {
    class GameBoy;
}; // namespace Core

namespace Memory {

class MemoryPage
{
    u16 base;
    u32 size;
    std::vector<u8> bytes;
public:
    MemoryPage(u16 base, u32 size)
    : base(base),
      size(size),
      bytes(size) {}

    u16 GetBase() { return base; }
    u32 GetSize() { return size; }
    std::vector<u8>& GetBytes() { return bytes; }
    u8* GetRaw() { return bytes.data(); }
};

class MBC
{
protected:
    Core::GameBoy* gameboy;

    std::unique_ptr<MemoryPage> romBank0;
    std::unique_ptr<MemoryPage> romBank1;
    std::unique_ptr<MemoryPage> vram;
    std::unique_ptr<MemoryPage> sram;
    std::unique_ptr<MemoryPage> wram;
    std::unique_ptr<MemoryPage> oam;
    std::unique_ptr<MemoryPage> highRam;

public:
    MBC(Core::GameBoy* gameboy);

    virtual std::unique_ptr<MemoryPage>& GetPage(u16 address);

    virtual void Write8(u16 address, u8 data);
    virtual void Write16(u16 address, u16 data);
    virtual u8 Read8(u16 address);
    virtual u16 Read16(u16 address);

    virtual void WriteBytes(const u8* src, u16 destination, u16 size);
    virtual void ReadBytes(u8* destination, u16 src, u16 size);
};

}; // namespace Memory
