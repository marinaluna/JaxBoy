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

#include <memory>


namespace Core {

class GameBoy;
class MemoryMap;

class PPU
{
    friend class GameBoy;

    enum class DisplayMode : int
    {
        HBlank,
        VBlank,
        OAMAccess,
        Update
    };

    // IO Registers
    // LCD controller
    u8 LCDC;
    // Enabled interrupts
    u8 InterruptFlags;
    DisplayMode Mode;
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
    // Background map for Tileset; 1 byte per pixel
    std::vector<u8> tileset;
    // cycle counter per frame
    int frameCycles;

    int lcd_width;
    int lcd_height;

    // system pointers
    GameBoy* gameboy;
    std::shared_ptr<MemoryMap> memory_map;

public:
    PPU(GameBoy* gameboy, int width, int height, std::shared_ptr<MemoryMap>& memory_map);
    ~PPU();

    int Tick(int cycles);

    void DrawFrame();
    void DecodeTile(u8 tileID);
    void UpdateTileset();
};

}; // namespace Core
