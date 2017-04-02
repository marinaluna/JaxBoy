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
#include <memory>


namespace Debug {
    class Logger;
}; // namespace Debug

namespace Graphics {
    struct Tile
    {
        // pixels are stored every two bits
        // 16 bits per row for 8 rows
        u16 rows[8];

        inline void Decode(const u8 src[16])
        {
            // interleave the bits
            for(int row = 0; row < 8; row++)
            {
                u32 upper = static_cast<u32>(src[(row*2)+1]) << 16;
                u32 lower = static_cast<u32>(src[row*2]);
                u32 morton = upper | lower;
                morton = (morton ^ (morton << 4)) & 0x0F0F0F0F; // separate nybbles
                morton = (morton ^ (morton << 2)) & 0x33333333; // separate half-nybbles
                morton = (morton ^ (morton << 1)) & 0x55555555; // separate bits
                rows[row] = static_cast<u16>(morton | (morton >> 15));
            }
        }

        inline const u8 GetPixel(u8 x, u8 y)
        {
            // mask out the two bits for the pixel we want
            // then shift it back to the bottom for an
            // array index
            x *= 2;
            return static_cast<u8>((rows[y] & (0xC000 >> x)) >> (14 - x));
        }
    };
}; // namespace Graphics

namespace Core {

class GameBoy;
class MemoryMap;

class PPU
{
    friend class GameBoy;

    // IO Registers
    // LCD controller
    u8 LCDC;
    // LCD Status
    u8 STAT;
    u8 ScrollY, ScrollX;
    u8 Line;
    // Acts as a breakpoint
    u8 LineCompare;
    // Palettes
    Color BGPalette[4];
    Color OBJ1Palette[4];
    Color OBJ2Palette[4];
    // Position of the Window, X is minus 7
    u8 WindowY, WindowX;

    // Framebuffer of pixels that draw on the screen
    std::vector<Color> framebuffer;
    // Spritesheets
    std::vector<Graphics::Tile> BGTileset;
    std::vector<Graphics::Tile> OBJTileset;
    // cycle counter per frame
    int frameCycles;

    int lcd_width;
    int lcd_height;

    // system pointers
    GameBoy* gameboy;
    std::shared_ptr<MemoryMap> memory_map;

    std::shared_ptr<Debug::Logger> logger;

public:
    PPU(GameBoy* gameboy, int width, int height, std::shared_ptr<MemoryMap>& memory_map, std::shared_ptr<Debug::Logger>& logger);
    ~PPU();

    int Tick(int cycles);

    void DrawFrame();
    void DecodeTiles();
};

}; // namespace Core
