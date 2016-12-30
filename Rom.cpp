#include <cstdio>

#include "Rom.h"


Rom::Rom(const char* rom_name)
:rom_name(rom_name)
{
	printf("Loaded ROM image: %s\n", rom_name);
}
