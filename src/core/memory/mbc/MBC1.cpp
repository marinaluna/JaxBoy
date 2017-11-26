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

#include "MBC1.h"

#include "../../GameBoy.h"
#include "../../Rom.h"

#include <cmath>
#include <string>


namespace Memory {

MBC1::MBC1(Core::GameBoy* gameboy)
:   MBC(gameboy)
{
    romBank = 0x01;
    extRamEnabled = false;
    ramBanking = false;
    selectedBank = 0x00;
}

void MBC1::Load(std::unique_ptr<Core::Rom>& rom)
{
    WriteBytes(rom->GetBytes().data(), 0x0000, 0x4000);
    // initialize all ROM banks
    u16 romSize = 32 * pow(2, rom->GetROMSize());
    u8 banks = romSize / 16; 

    numBanks = banks;

    for(int i = 1; i < banks; i++) {
        switchableBanks.push_back(std::unique_ptr<MemoryPage>(new MemoryPage(0x4000, 0x4000)));

        std::memcpy(switchableBanks.at(i-1)->GetRaw(), rom->GetBytes().data() + (0x4000*i), 0x4000);
    }
    for(int i = 0; i < 4; i++) {
        ramBanks[i] = std::unique_ptr<MemoryPage>(new MemoryPage(0xA000, 0x2000));
    }
}

std::unique_ptr<MemoryPage>& MBC1::GetPage(u16 address)
{
    if(address >= 0x4000 && address <= 0x7FFF) {
        if(!ramBanking)
            return switchableBanks.at(romBank - 1 | (selectedBank << 5));
        else
            return switchableBanks.at(romBank - 1);
    }
    if(address >= 0xA000 && address <= 0xBFFF) {
        if(!extRamEnabled || !ramBanking)
            return ramBanks[0x00];
        else
            return ramBanks[selectedBank];
    }

    return MBC::GetPage(address);
}

void MBC1::Write8(u16 address, u8 data)
{
    if(address >= 0x0000 && address <= 0x1FFF)
    {
        if((data&0xF) == 0xA)
            extRamEnabled = true;
        else
            extRamEnabled = false;
        return;
    }
    if(address >= 0x2000 && address <= 0x3FFF)
    {
        // bug
        if(data == 0x00 || data == 0x20 || data == 0x40 || data == 0x60)
            data++;
        romBank = data & 0x1F;
        return;
    }
    if(address >= 0x4000 && address <= 0x5FFF)
    {
        selectedBank = data;
        return;
    }
    if(address >= 0x6000 && address <= 0x7FFF)
    {
        if(data == 0x00)
            ramBanking = false;
        else
            ramBanking = true;
        return;
    }
    MBC::Write8(address, data);
}

}; // namespace Memory