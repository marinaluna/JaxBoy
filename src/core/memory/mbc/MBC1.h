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
#include "MBC.h"


namespace Memory {

class MBC1
: public MBC
{
    std::vector<std::unique_ptr<MemoryPage>> switchableBanks;
    u8 romBank;
    u8 numBanks;
    bool extRamEnabled;
    bool ramBanking;
    std::unique_ptr<MemoryPage> ramBanks[0x04];
    u8 selectedBank;

public:
    MBC1(Core::GameBoy* gameboy);
    virtual void Load(std::unique_ptr<Core::Rom>& rom);

    virtual std::unique_ptr<MemoryPage>& GetPage(u16 address);

    virtual void Write8(u16 address, u8 data);
};

}; // namespace Memory
