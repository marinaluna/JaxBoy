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

#include "Globals.h"
#include "Types.h"

#include "../external/MiniFB/MiniFB.h"


const char* gAppName = "JaxBoy";

u8 gBitMasks[8] = 
{
    0b00000001,
    0b00000010,
    0b00000100,
    0b00001000,
    0b00010000,
    0b00100000,
    0b01000000,
    0b10000000
};

const Color gColors[4] =
{
    // White
    MFB_RGB(0x9B, 0xBC, 0x0F),
    // Light Grey
    MFB_RGB(0x8B, 0xAC, 0x0F),
    // Dark Grey
    MFB_RGB(0x30, 0x62, 0x30),
    // Black
    MFB_RGB(0x0F, 0x38, 0x0F)
};
