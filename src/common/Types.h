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

#include <cstdint>

using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;

using s8 = int8_t;
using s16 = int16_t;
using s32 = int32_t;

using Color = u32;

using Reg8 = u8;
union Reg16
{
    u16 word;

    struct
    {
        Reg8 low;
        Reg8 high;
    };
};

// Display Modes
enum
{
    DISPLAY_HBLANK,
    DISPLAY_VBLANK,
    DISPLAY_OAMACCESS,
    DISPLAY_UPDATE
};
