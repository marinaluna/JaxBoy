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

#include "MBC3.h"

#include "../../GameBoy.h"

#include <string>


namespace Memory {

MBC3::MBC3(Core::GameBoy* gameboy)
:   MBC1(gameboy) {
    // TODO: RTC, Battery
}

std::unique_ptr<MemoryPage>& MBC3::GetPage(u16 address)
{
    if(address >= 0x4000 && address <= 0x7FFF) {
        return switchableBanks.at(romBank - 1);
    }

    return MBC1::GetPage(address);
}

void MBC3::Write8(u16 address, u8 data)
{
    if(address >= 0x2000 && address <= 0x3FFF) {
        // bug
        if((data & 0x7F) == 0x00)
            data++;
        romBank = data & 0x7F;
        return;
    }

    MBC1::Write8(address, data);
}

u8 MBC3::Read8(u16 address)
{
    if(address >= 0xA000 && address <= 0xBFFF) {
        if(selectedBank & 0x08) {
            return 0xFF;
        } else {
            try {
                return ramBanks[selectedBank]->GetBytes().at(address - 0xA000);
            }
            catch(std::out_of_range& e) {
                gameboy->SystemError("Memory read out of range!");
                return 0xFF;
            }
        }
    }

    return MBC1::Read8(address);
}

}; // namespace Memory