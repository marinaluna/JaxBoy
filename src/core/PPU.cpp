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

PPU::PPU(GameBoy* gameboy, int width, int height, std::shared_ptr<MemoryMap>& memory_map, std::shared_ptr<Debug::Logger>& logger)
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
    BGTileset = std::vector<Graphics::Tile>(256);
    OBJTileset = std::vector<Graphics::Tile>(256);
    // Start in DISPLAY_UPDATE
    STAT |= DISPLAY_UPDATE;
    // Setup a blank palette
    BGPalette[0] = BGPalette[1] = BGPalette[2] = BGPalette[3] = gColors[0x00];

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
        switch(STAT & 0x03)
        {
            case DISPLAY_HBLANK:
                // TODO: Accurate cycles?
                if(frameCycles > 207)
                {
                    // Carry leftover cycles into next mode
                    frameCycles %= 207;
                    if(++Line == 144)
                    {
                        // At the last line; enter V-Blank
                        STAT = (STAT & ~0x03) | DISPLAY_VBLANK;
                        // request V-Blank interrupt
                        memory_map->Write8(0xFF0F, memory_map->Read8(0xFF0F) | 0b00000001);
                    }
                    else
                    {
                        // Proceed to the next line
                        STAT = (STAT & ~0x03) | DISPLAY_OAMACCESS;
                    }
                }
                break;
            case DISPLAY_VBLANK:
                // Have we completed a scanline?
                if((floor(frameCycles / 465) + 144) > Line)
                {
                    if(++Line > 153)
                    {
                        frameCycles %= 4560;
                        STAT = (STAT & ~0x03) | DISPLAY_OAMACCESS;
                        Line = 0;
                        // Redraw the frame after V-Blank
                        DrawFrame();
                        return_code = mfb_update(framebuffer.data());
                    }
                }
                break;
            case DISPLAY_OAMACCESS:
                if(frameCycles > 83)
                {
                    frameCycles %= 83;
                    STAT = (STAT & ~0x03) | DISPLAY_UPDATE;
                }
                break;
            case DISPLAY_UPDATE:
                if(frameCycles > 175)
                {
                    frameCycles %= 175;
                    STAT = (STAT & ~0x03) | DISPLAY_HBLANK;
                    DecodeTiles();
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

    return return_code;
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
                    framebuffer[(((y * 8) + py + 24) * lcd_width) + ((x * 8) + px + 1)] = BGPalette[BGTileset[tileID].GetPixel(px, py+scYpixel)];
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
                        framebuffer[(((y * 8) + tophalf + ny + 24) * lcd_width) + ((x * 8) + px + 1)] = BGPalette[BGTileset[tileID].GetPixel(px, ny)];
                    }
                }
            }
        }
    }
}

void PPU::DecodeTiles()
{
    // fetch each tile in the tileset from VRAM
    // and decode their 2-bit colors
    const int BG_SIZE = 16;
    const int OBJ_SIZE = 16;

    // Background Tileset
    for(int bg = 0; bg < 256; bg++)
    {
        // if LCDC bit 4 == 0,
        // BG tiles 00-7F start at 0x9000
        // and 80-FF start at 0x8800
        // Otherwise, BG tiles align with
        // OBJ tiles at 0x8000
        u16 base = 0x8000;
        u8 rawTile = bg;
        if((LCDC & 0x10) == 0)
        {
            if(bg >= 128)
            {
                base = 0x8800;
                // start fetching from 0x8800,
                // not 0x8800 + 128 tiles
                rawTile -= 128;
            }
            else
                base = 0x9000;
        }

        u8 buffer[BG_SIZE];
        memory_map->CopyBytes(buffer, base + (rawTile * BG_SIZE), BG_SIZE);
        BGTileset.at(bg).Decode(buffer);
    }
    // Sprite tileset
    for(int obj = 0; obj < 256; obj++)
    {
        u8 buffer[OBJ_SIZE];
        memory_map->CopyBytes(buffer, 0x8000 + (obj * OBJ_SIZE), OBJ_SIZE);
        OBJTileset.at(obj).Decode(buffer);
    }
}

}; // namespace Core
