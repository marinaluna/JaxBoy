#include "GameBoy.h"
#include "util/Macros.h"

#include "MiniFB/MiniFB.h"

#include <cmath> // for floor

#include "PPU.h"

// Padding to accommodate for window toolbar
#define WINDOW_WIDTH	(160 + 2)
#define WINDOW_HEIGHT	(144 + 25)

const Color LCD_WHITE = MFB_RGB(0x9B, 0xBC, 0x0F);
const Color LCD_LIGHT = MFB_RGB(0x8B, 0xAC, 0x0F);
const Color LCD_DARK = MFB_RGB(0x30, 0x62, 0x30);
const Color LCD_BLACK = MFB_RGB(0x0F, 0x38, 0x0F);
const Color Colors[4] = {LCD_WHITE, LCD_LIGHT, LCD_DARK, LCD_BLACK};
#define COLOR_TO_2BIT(color) \
		(color == LCD_WHITE? 0b00 : (color == LCD_LIGHT? 0b01 : (color == LCD_DARK? 0b10 : (color == LCD_BLACK? 0b11 : LCD_WHITE))))


// 1 byte per pixel 00 01 10 11
uint8_t PPU::BGMap[128][8*8];
PPU::PPU()
{
	// initialize the LCD
	mfb_open("JaxBoy", WINDOW_WIDTH, WINDOW_HEIGHT);
	framebuffer = new Color[WINDOW_WIDTH * WINDOW_HEIGHT];
	// Setup a blank palette
	BGPalette[0] = BGPalette[1] = BGPalette[2] = BGPalette[3] = LCD_WHITE;

	DisplayMode = 0x02;

	// Update the window once to create it
	mfb_update(framebuffer);
}

PPU::~PPU()
{
	mfb_close();
	if(framebuffer != nullptr)
		delete framebuffer;
}

int PPU::Tick(int cycles)
{
	int return_code = 0;
	if(Control.enabled)
	{
		frameCycles += cycles;
		switch(DisplayMode)
		{
		case 0x00: // H-Blank
		{
			// TODO: Accurate cycles?
			if(frameCycles > 207)
			{
				if(++Line == 144)
				{
					// At the last line; enter V-Blank
					DisplayMode = 0x1;
				}
				else
				{
					// Proceed to the next line
					DisplayMode = 0x02;
				}
				// Carry leftover cycles into next mode
				frameCycles %= 207;
			}
			break;
		}
		case 0x01: // V-Blank
		{
			// Have we completed a scanline?
			if((floor(frameCycles / 465) + 144) > Line)
			{
				if(++Line > 153)
				{
					DisplayMode = 0x02;
					frameCycles %= 4560;
					Line = 0;

					// Redraw the frame after V-Blank
					// TODO: Is this the best place to do this?
					UpdateFrame();
					return_code = mfb_update(framebuffer);
				}
			}
			break;
		}
		case 0x02: // OAM Access
		{
			if(frameCycles > 83)
			{
				DisplayMode = 0x03;
				frameCycles %= 83;
			}
			break;
		}
		case 0x03: // Update
		{
			if(frameCycles > 175)
			{
				DecodeBGMap();
				//DrawScanline();
				DisplayMode = 0x00;
				frameCycles %= 175;
			}
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

void PPU::WriteRegister(uint16_t address, uint8_t data)
{
	switch(address)
	{
		case 0xFF40:
			Control = data;
			break;
		case 0xFF41:
			InterruptFlags={static_cast<bool>(data&0b1000),
							static_cast<bool>(data&0b10000),
							static_cast<bool>(data&0b100000),
							static_cast<bool>(data&0b1000000)};
			// Note: the other flags are read only
			break;
		case 0xFF42:
			ScrollY = data;
			break;
		case 0xFF43:
			ScrollX = data;
			break;
		case 0xFF44:
			// Read only; writing resets this;
			Line = 0;
			break;
		case 0xFF45:
			Line_Interrupt = data;
			break;
		case 0xFF47:
			BGPalette[0] = Colors[(data&0b00000011)];
			BGPalette[1] = Colors[(data&0b00001100)>>2];
			BGPalette[2] = Colors[(data&0b00110000)>>4];
			BGPalette[3] = Colors[(data&0b11000000)>>6];
			break;
		// For Sprite palettes 00 is transparent
		case 0xFF48:
			OBJ1Palette[0] = Colors[(data&0b00000011)];
			OBJ1Palette[1] = Colors[(data&0b00001100)>>2];
			OBJ1Palette[2] = Colors[(data&0b00110000)>>4];
			OBJ1Palette[3] = Colors[(data&0b11000000)>>6];
			break;
		case 0xFF49:
			OBJ2Palette[0] = Colors[(data&0b00000011)];
			OBJ2Palette[1] = Colors[(data&0b00001100)>>2];
			OBJ2Palette[2] = Colors[(data&0b00110000)>>4];
			OBJ2Palette[3] = Colors[(data&0b11000000)>>6];
			break;
		case 0xFF4A:
			WindowY = data;
			break;
		case 0xFF4B:
			WindowX = data;
	}
}

uint8_t PPU::ReadRegister(uint16_t address)
{
	switch(address)
	{
		case 0xFF40:
			return Control;
		case 0xFF41:
			return(	(InterruptFlags.ILYC << 6) |
					(InterruptFlags.IOAM << 5) |
					(InterruptFlags.IVBlank << 4) |
					(InterruptFlags.IHBlank << 3) |
					((Line == Line_Interrupt) << 2) |
					(DisplayMode));
		case 0xFF42:
			return ScrollY;
		case 0xFF43:
			return ScrollX;
		case 0xFF44:
			return Line;
		case 0xFF45:
			return Line_Interrupt;
		case 0xFF47:
			// These are really hard to convert back as it turns out
			return(	(COLOR_TO_2BIT(BGPalette[3]) << 6) |
					(COLOR_TO_2BIT(BGPalette[2]) << 4) |
					(COLOR_TO_2BIT(BGPalette[1]) << 2) |
					(COLOR_TO_2BIT(BGPalette[0])));
		case 0xFF48:
			return(	(COLOR_TO_2BIT(OBJ1Palette[3]) << 6) |
					(COLOR_TO_2BIT(OBJ1Palette[2]) << 4) |
					(COLOR_TO_2BIT(OBJ1Palette[1]) << 2) |
					(COLOR_TO_2BIT(OBJ1Palette[0])));
		case 0xFF49:
			return(	(COLOR_TO_2BIT(OBJ2Palette[3]) << 6) |
					(COLOR_TO_2BIT(OBJ2Palette[2]) << 4) |
					(COLOR_TO_2BIT(OBJ2Palette[1]) << 2) |
					(COLOR_TO_2BIT(OBJ2Palette[0])));
		case 0xFF4A:
			return WindowY;
		case 0xFF4B:
			return WindowX;
	}

	return 0x00;
}

void PPU::UpdateFrame()
{
	// Converts the GameBoy's old-fashioned 8-bit framebuffer to
	// our modern high-tech fr4m3buff3r 2000

	// TODO: This code is *terrible*
	for(int y = 0; y < 18; y++)
	{
		for(int x = 0; x < 20; x++)
		{
			uint8_t scYtile = floor(ScrollY / 8);
			// TODO: This grabs pixels to the wrong tile with the current code,
			// fix this by more accurately calculating the Tile to use
			uint8_t scYpixel = 0;//(ScrollY % 8);
			uint8_t tileID = GameBoy::Read8(0x9800 + (((y + scYtile) * 32) + x));

			for(int py = 0; py < 8; py++)
			{
				for(int px = 0; px < 8; px++)
				{
					// TODO: math iz hurd
					framebuffer[(((y * 8) + py + 24) * WINDOW_WIDTH) + ((x * 8) + px + 1)] = BGPalette[BGMap[tileID][((py + scYpixel) * 8) + px]];
				}
			}
		}
	}
}

void PPU::DecodeBGMap()
{
	// TODO: This code is terrible too, but it works
	const int tile_count = 128;
	const int tile_size = 16;
	const int bytes = tile_count * tile_size;

	uint8_t* buffer = new uint8_t[bytes];
	for(int i = 0; i < bytes; i++)
	{
		buffer[i] = GameBoy::Read8(0x8000 + i);
	}

	// 16 bytes per 8x8 tile
	for(int i = 0; i < bytes; i += tile_size)
	{
		// 2 bytes per row
		for(int j = 0; j < tile_size; j += 2)
		{
			int row = j / 2;
			uint8_t top = buffer[i + j];
			uint8_t bottom = buffer[i + j + 1];

			// First byte holds the row's top color bits
			// Second byte holds the row's bottom color bits
			BGMap[i / tile_size][(row * 8) + 0] = ((top & BIT7_MASK) >> 6) | ((bottom & BIT7_MASK) >> 7);
			BGMap[i / tile_size][(row * 8) + 1] = ((top & BIT6_MASK) >> 5) | ((bottom & BIT6_MASK) >> 6);
			BGMap[i / tile_size][(row * 8) + 2] = ((top & BIT5_MASK) >> 4) | ((bottom & BIT5_MASK) >> 5);
			BGMap[i / tile_size][(row * 8) + 3] = ((top & BIT4_MASK) >> 3) | ((bottom & BIT4_MASK) >> 4);
			BGMap[i / tile_size][(row * 8) + 4] = ((top & BIT3_MASK) >> 2) | ((bottom & BIT3_MASK) >> 3);
			BGMap[i / tile_size][(row * 8) + 5] = ((top & BIT2_MASK) >> 1) | ((bottom & BIT2_MASK) >> 2);
			BGMap[i / tile_size][(row * 8) + 6] = ((top & BIT1_MASK) >> 0) | ((bottom & BIT1_MASK) >> 1);
			BGMap[i / tile_size][(row * 8) + 7] = ((top & BIT0_MASK) << 1) | ((bottom & BIT0_MASK) >> 0);
		}
	}

	delete[] buffer;
}
