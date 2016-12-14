#include <cstdio>

#include "Bootrom.h"


Bootrom::Bootrom(const char* bootrom_name)
:Rom(bootrom_name)
{
	FILE* rom = fopen(rom_name, "rb");

	fclose(rom);
}
