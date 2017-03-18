#pragma once

#include <cstdint>

// Colors for the LCD
typedef uint32_t Color;
extern const Color LCD_WHITE;
extern const Color LCD_LIGHT;
extern const Color LCD_DARK;
extern const Color LCD_BLACK;

class PPU
{
	Color* framebuffer;
public:

	PPU();
	~PPU();

	int Tick();
};
