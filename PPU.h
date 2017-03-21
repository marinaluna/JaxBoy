#pragma once
#include "util/Macros.h"

#include <cstdint>

// Colors for the LCD
typedef uint32_t Color;
extern const Color LCD_WHITE;
extern const Color LCD_LIGHT;
extern const Color LCD_DARK;
extern const Color LCD_BLACK;
extern const Color Colors[4];

struct LCDC
{
    bool enabled;
    uint16_t window_area;
    bool window_enabled;
    uint16_t sprite_map_area;
    uint16_t bg_area;
    bool large_sprites;
    bool display_sprites;
    bool display_bg;

    inline const LCDC& operator=(const uint8_t raw)
    {
        enabled =          CHECKBIT(BIT7_MASK, raw) != 0;
        window_area =      (CHECKBIT(BIT6_MASK, raw) == 0)? 0x9800 : 0x9C00;
        window_enabled =   CHECKBIT(BIT5_MASK, raw) != 0;
        sprite_map_area =  (CHECKBIT(BIT4_MASK, raw) == 0)? 0x8800 : 0x8000;
        bg_area =          (CHECKBIT(BIT3_MASK, raw) == 0)? 0x9800 : 0x9C00;
        large_sprites =    CHECKBIT(BIT2_MASK, raw) != 0;
        display_sprites =  CHECKBIT(BIT1_MASK, raw) != 0;
        display_bg =       CHECKBIT(BIT0_MASK, raw) != 0;

        return *this;
    }
    inline operator uint8_t()
    {
        return( (display_bg << 0) |
                (display_sprites << 1) |
                (large_sprites << 2) |
                ((bg_area == 0x9C00? 1 : 0) << 3) |
                ((sprite_map_area == 0x8000? 1 : 0) << 4) |
                (window_enabled << 5) |
                ((window_area == 0x9C00? 1 : 0) << 6) |
                (enabled << 7));
    }
};

class PPU
{
    struct Interrupts{
        bool IHBlank, IVBlank, IOAM, ILYC;
    };

    // High-Level registers
    LCDC Control;
    // Enabled interrupts
    Interrupts InterruptFlags;
    uint8_t DisplayMode;
    uint8_t ScrollY, ScrollX;
    uint8_t Line;
    // Acts as a breakpoint
    uint8_t Line_Interrupt;
    // Palettes
    Color BGPalette[4];
    Color OBJ1Palette[4];
    Color OBJ2Palette[4];
    // Position of the Window, X is minus 7
    uint8_t WindowY, WindowX;

    Color* framebuffer;
    int frameCycles;

    static uint8_t BGMap[128][8*8];
public:

    PPU();
    ~PPU();

    int Tick(int cycles);

    void WriteRegister(uint16_t address, uint8_t data);
    uint8_t ReadRegister(uint16_t address);

    void UpdateFrame();
    void DecodeBGMap();
};
