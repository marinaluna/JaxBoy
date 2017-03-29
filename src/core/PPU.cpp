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

#include "PPU.h"
#include "memory/MemoryMap.h"

#include "../common/Globals.h"

#include "../external/MiniFB/MiniFB.h"

#include <memory>
#include <cmath>


namespace Core {

PPU::PPU(GameBoy* gameboy, int width, int height, std::shared_ptr<MemoryMap>& memory_map, std::shared_ptr<Debugger::Logger>& logger)
:
    gameboy (gameboy),
    lcd_width (width),
    lcd_height (height),
    memory_map (memory_map),
    logger (logger)
{
    // initialize the LCD
    mfb_open(gAppName, width, height);

    // initialize buffers
    framebuffer = std::vector<Color>(width * height);
    tileset = std::vector<u8>(128 * 8*8);
    // Setup a blank palette
    BGPalette[0] = BGPalette[1] = BGPalette[2] = BGPalette[3] = gColors[0x00];
    Mode = DisplayMode::Update;

    // Update the window once to create it
    mfb_update(framebuffer.data());
}

PPU::~PPU()
{
    mfb_close();
}

int PPU::Tick(int cycles)
{
    int return_code = 0;
    if((LCDC & 0b10000000) != 0)
    {
        frameCycles += cycles;
        switch(Mode)
        {
            case DisplayMode::HBlank:
                // TODO: Accurate cycles?
                if(frameCycles > 207)
                {
                    // Carry leftover cycles into next mode
                    frameCycles %= 207;
                    if(++Line == 144)
                    {
                        // At the last line; enter V-Blank
                        Mode = DisplayMode::VBlank;
                    }
                    else
                    {
                        // Proceed to the next line
                        Mode = DisplayMode::OAMAccess;
                    }
                }
                break;
            case DisplayMode::VBlank:
                // Have we completed a scanline?
                if((floor(frameCycles / 465) + 144) > Line)
                {
                    if(++Line > 153)
                    {
                        frameCycles %= 4560;
                        Mode = DisplayMode::OAMAccess;
                        Line = 0;
                        // Redraw the frame after V-Blank
                        DrawFrame();
                        return_code = mfb_update(framebuffer.data());
                    }
                }
                break;
            case DisplayMode::OAMAccess:
                if(frameCycles > 83)
                {
                    frameCycles %= 83;
                    Mode = DisplayMode::Update;
                }
                break;
            case DisplayMode::Update:
                if(frameCycles > 175)
                {
                    frameCycles %= 175;
                    Mode = DisplayMode::HBlank;
                    UpdateTileset();
                }
                break;
        }
    }
    else
    {
        // If LCDC is disabled, reset all this stuff
        frameCycles = 0;
        Line = 0;
    }

    return 0;
}

void PPU::DrawFrame()
{
    // TODO: rewrite this
    for(int y = 0; y < 18; y++)
    {
        for(int x = 0; x < 20; x++)
        {
            u8 scYtile = floor(ScrollY / 8);
            u8 scYpixel = (ScrollY % 8);
            u8 tophalf = (8 - scYpixel);
            u8 tileID;

            for(int py = 0; py < tophalf; py++)
            {
                for(int px = 0; px < 8; px++)
                {
                    tileID = memory_map->Read8(0x9800 + (((y + scYtile) * 32) + x));
                    framebuffer[(((y * 8) + py + 24) * lcd_width) + ((x * 8) + px + 1)] = BGPalette[tileset[(tileID * 64) + ((py + scYpixel) * 8) + px]];
                }
            }
            // If there's a scroll,
            // complete the bottom half of the tile using
            // the image of the next tile down
            if(tophalf != 0)
            {
                // fetch the next tile down
                tileID = memory_map->Read8(0x9800 + (((y + scYtile + 1) * 32) + x));
                for(int ny = 0; ny < scYpixel; ny++)
                {
                    for(int px = 0; px < 8; px++)
                    {
                        // Somehow this works
                        framebuffer[(((y * 8) + tophalf + ny + 24) * lcd_width) + ((x * 8) + px + 1)] = BGPalette[tileset[(tileID * 64) + (ny * 8) + px]];
                    }
                }
            }
        }
    }
}

void PPU::DecodeTile(u8 tileID)
{
    // each tile is 16 bytes
    u8* buffer = new u8[16];
    for(int i = 0; i < 16; i++)
    {
        buffer[i] = memory_map->Read8(0x8000 + (16 * tileID) + i);
    }

    // iterate through the rows
    for(int y = 0; y < 16; y += 2)
    {
        const int row = y / 2;
        const u8 upperByte = buffer[y];
        const u8 lowerByte = buffer[y + 1];
        // iterate through the columns
        for(int x = 0; x < 8; x++)
        {
            // the top row determines each color's top bit
            tileset[(tileID * 64) + (row * 8) + x]  = ((upperByte & (0b10000000 >> x)) != 0)? 0b10 : 0b00;
            // the bottom row determines each color's bottom bit
            tileset[(tileID * 64) + (row * 8) + x] |= ((lowerByte & (0b10000000) >> x) != 0)? 0b01 : 0b00;
        }
    }

    delete[] buffer;
}

void PPU::UpdateTileset()
{
    // fetch each tile in the tileset from VRAM
    // and decode their 2-bit colors
    for(int i = 0; i < 128; i++)
    {
        DecodeTile(i);
    }
}

}; // namespace Core
