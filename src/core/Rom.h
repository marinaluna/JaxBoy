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

#include <vector>


namespace Debug {
    class Logger;
}; // namespace Debug

namespace Core {

class GameBoy;

class Rom
{
    struct Header
    {
        char Name[16];
        char Manufacturer[4];
        bool UsesSGBFeatures;
        u8 CartType;
        u8 RomSize;
        u8 RamSize;
        bool International;
        u8 Licensee;
        u8 VersionCode;
    };
    Header header;
    // all bytes in the ROM
    std::vector<u8> bytes;
    // Logger instance for ROM
    std::shared_ptr<Debug::Logger> logger;

public:
    Rom(const std::vector<u8>& bytes,
        const std::shared_ptr<Debug::Logger>& logger);

    std::vector<u8>& GetBytes()
        { return bytes; }
};

}; // namespace Core
