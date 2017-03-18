#include "MiniFB/MiniFB.h"

#include "PPU.h"

#define WINDOW_PADDING	 30
// Includes 30 pixels of padding to accommodate for window toolbar
#define WINDOW_WIDTH	(190 + WINDOW_PADDING)
#define WINDOW_HEIGHT	(174 + WINDOW_PADDING)

const Color LCD_WHITE = MFB_RGB(0x9B, 0xBC, 0x1B);
const Color LCD_LIGHT = MFB_RGB(0x8B, 0xAC, 0x17);
const Color LCD_DARK = MFB_RGB(0x30, 0x62, 0x30);
const Color LCD_BLACK = MFB_RGB(0x16, 0x38, 0x0F);


PPU::PPU()
{
	// initialize the LCD
	mfb_open("Gameboy Emulator", WINDOW_WIDTH, WINDOW_HEIGHT);

	// Initialze the frame buffer with a blank screen
	framebuffer = new Color[WINDOW_WIDTH * WINDOW_HEIGHT];
	for(int i = 0; i < WINDOW_WIDTH * WINDOW_HEIGHT; i++)
	{
		framebuffer[i] = LCD_WHITE;
	}

	// Update the window once to create it
	mfb_update(framebuffer);
}

PPU::~PPU()
{
	mfb_close();
	if(framebuffer != nullptr)
		delete framebuffer;
}

int PPU::Tick()
{
	return mfb_update(framebuffer);
}
