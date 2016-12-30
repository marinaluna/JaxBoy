#pragma once

class Rom;
class Processor;
struct MemoryMap;

class GameBoy
{
	Processor* processor;
	MemoryMap* memory_map;
	Rom* loaded_rom;

	static GameBoy* instance;
public:

	static GameBoy* GetGameBoy();
	GameBoy(const char* bootrom_path, const char* rom_name);

	void Start();
	void LoadBootrom(const char* bootrom_path);

	static MemoryMap& GetMemory();
	static Processor& GetCPU();

	static void SystemError(const char*, ...);
};
