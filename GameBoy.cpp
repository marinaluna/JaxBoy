#include "GameCart.h"
#include "Processor.h"
#include "PPU.h"
#include "constants.h"

#include "memory/MemoryMap.h"
#include "memory/MemoryRegion.h"

#include <cstdio>
#include <cstdlib>
#include <cstdarg>

#include "GameBoy.h"


bool GameBoy::IsDebugMode = false;

GameBoy* GameBoy::instance;
GameBoy* GameBoy::GetGameBoy()
{
	return instance;
}

GameBoy::GameBoy(const char* bootrom_path, const char* rom_name)
{
	// TODO: add this to boot args
	IsDebugMode = true;

	instance = this;

	memory_map = new MemoryMap(MEMORY_MAP_SIZE);
	processor = new Processor();
	ppu = new PPU();

	Stopped = false;

	// Loads external bootrom binary for now
	LoadBootrom(bootrom_path);
}

GameBoy::~GameBoy()
{
	if(ppu != nullptr)
		delete ppu;
	if(processor != nullptr)
		delete processor;
	if(memory_map != nullptr)
		delete memory_map;
}

void GameBoy::Run()
{
	uint32_t i = 0;
	while(!Stopped)
	{
		int cycles = processor->Tick();
		
		// if Escape was pressed or the Window was closed
		if(ppu->Tick() == -1)
		{
			Stop();
		}
	}
}

void GameBoy::Stop()
{
	printf("\n\n\nExiting GameBoy...\n\n\n");
	Stopped = true;
}

void GameBoy::LoadBootrom(const char* bootrom_path)
{
	FILE* file = fopen(bootrom_path, "rb");
	
	uint8_t* buffer = (uint8_t*) malloc(0x100);
	fread(buffer, 0x100, 1, file);

	GetMemory().GetRegionFromAddress(0x0000)->memcpy(0x0000, (void*) buffer, 0x100);

	free(buffer);
	fclose(file);
}

MemoryMap& GameBoy::GetMemory()
{
	return *GetGameBoy()->memory_map;
}

Processor& GameBoy::GetCPU()
{
	return *GetGameBoy()->processor;
}

void GameBoy::SystemError(const char* error_msg, ...)
{
	va_list args;
	va_start(args, error_msg);
	printf("ERROR: "); vprintf(error_msg, args);
	va_end(args);
	instance->Stop();
}
