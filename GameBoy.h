#pragma once

class Rom;
class Processor;
class PPU;
struct MemoryMap;

class GameBoy
{
	Processor* processor;
	PPU* ppu;
	MemoryMap* memory_map;
	Rom* loaded_rom;

	bool Stopped;

	static GameBoy* instance;
public:
	static bool IsDebugMode;

	static GameBoy* GetGameBoy();
	GameBoy(const char* bootrom_path, const char* rom_name);
	~GameBoy();

	void Run();
	void Stop();

	void LoadBootrom(const char* bootrom_path);

	static MemoryMap& GetMemory();
	static Processor& GetCPU();

	static void SystemError(const char*, ...);
};
