#pragma once

class Rom;
class Processor;
struct MemoryMap;

class GameBoy
{
public:
	static Processor* processor;
	static MemoryMap* memory_map;

	static Rom* loaded_bootrom;
	static Rom* loaded_rom;


	GameBoy(const char* bootrom_name, const char* rom_name);
};
