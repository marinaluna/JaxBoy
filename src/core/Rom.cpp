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

#include "Rom.h"

#include "../debug/Logger.h"

#include <string>
#include <algorithm>


namespace Core {

Rom::Rom(const std::vector<u8>& bytes)
:
    bytes (bytes)
{
    // copy the rom name (in newer carts the end of this is used by manufacturer code)
    std::copy(bytes.begin() + 0x134, bytes.begin() + 0x143, header.Name);
    LOG_MSG("Loaded rom: " + std::string(header.Name));
    // copy the new manufacturer code
    std::copy(bytes.begin() + 0x13F, bytes.begin() + 0x143, header.Manufacturer);
    header.UsesSGBFeatures = bytes.at(0x146) == 0x03; // 3 means yes, 0 means no
    header.RomSize = bytes.at(0x148);
    header.RamSize = bytes.at(0x149);
    header.International = bytes.at(0x14A) == 0x01; // 00 means Japan, 01 means international
    header.Licensee = bytes.at(0x14B); // if 33, SGB functions don't work
    header.VersionCode = bytes.at(0x14C); // usually 00

    // Cart Type specifies which MBC type is used in the cart,
    // and what external hardware (i.e. battery) is included.
    // The values are as follows:
    //     00 - ROM only                        15 - MBC4
    //     01 - MBC1                            16 - MBC4 + RAM
    //     02 - MBC1 + RAM                      17 - MBC4 + RAM + BATTERY
    //     03 - MBC1 + RAM + BATTERY            19 - MBC5
    //     05 - MBC2                            1A - MBC5 + RAM
    //     06 - MBC2 + BATTERY                  1B - MBC5 + RAM + BATTERY
    //     08 - ROM+RAM                         1C - MBC5 + RUMBLE
    //     09 - ROM + RAM + BATTERY             1D - MBC5 + RUMBLE + RAM
    //     0B - MMM01                           1E - MBC5 + RUMBLE + RAM + BATTERY
    //     0C - MMM01 + RAM                     20 - MBC6
    //     0D - MMM01 + RAM + BATTERY           22 - MBC7 + SENSOR + RUMBLE + RAM + BATTERY
    //     0F - MBC3 + TIMER + BATTERY
    //     10 - MBC3 + TIMER + RAM + BATTERY    FC - POCKET CAMERA
    //     11 - MBC3                            FD - BANDAI TAMA5
    //     12 - MBC3 + RAM                      FE - HuC3
    //     13 - MBC3 + RAM + BATTERY            FF - HuC1 + RAM + BATTERY
    header.CartType = bytes.at(0x147);
    //LOG_MSG("Cart type " + header.CartType);
}

}; // namespace Core
