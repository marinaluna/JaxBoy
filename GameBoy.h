#pragma once
#include "memory/MemoryMap.h"

#include <cstdint>

class Rom;
class Processor;
class PPU;

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
	static PPU& GetPPU();

	static void SystemError(const char*, ...);


	inline static void Write8(uint16_t address, uint8_t data)
	{
		GetMemory().Write8(address, data);
	}
	inline static void Write16(uint16_t address, uint16_t data)
	{
		GetMemory().Write16(address, data);
	}
	inline static uint8_t Read8(uint16_t address)
	{
		return GetMemory().Read8(address);
	}
	inline static uint16_t Read16(uint16_t address)
	{
		return GetMemory().Read16(address);
	}
};
