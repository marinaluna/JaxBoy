#include <cstdio>

#include "GameCart.h"


GameCart::GameCart(const char* rom_name)
:Rom(rom_name)
{
	// TODO
}

int GameCart::getSizeInBytes() const
{
	return 0x8000;
}

uint16_t GameCart::getStartAddress() const
{
	return 0x0000;
}

void GameCart::printDebugInfo() const
{
	printf("Loaded Game cart: %s\n", rom_name);
	//printf("ROM start address: %#04x\n", 0x0000);
}
