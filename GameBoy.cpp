#include "Bootrom.h"
#include "GameCart.h"
#include "MemoryMap.h"

#include "modules/Processor.h"

#include "GameBoy.h"


Processor* GameBoy::processor;
MemoryMap* GameBoy::memory_map;
Rom* GameBoy::loaded_bootrom;
Rom* GameBoy::loaded_rom;

GameBoy::GameBoy(const char* bootrom_name, const char* rom_name)
{
	processor = new Processor();
	memory_map = new MemoryMap(0x10000);

	loaded_bootrom = new Bootrom(bootrom_name);
	loaded_rom = new GameCart(rom_name);
}
