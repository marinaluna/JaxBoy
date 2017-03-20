#include "GameCart.h"
#include "Processor.h"
#include "PPU.h"

#include "util/Macros.h"

#include "memory/MemoryMap.h"
#include "memory/MemoryRegion.h"

#include <cstdio> // for fread
#include <cstdarg> // for va

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

	processor = new Processor();
	ppu = new PPU();
	memory_map = new MemoryMap(MEMORY_MAP_SIZE);

	Stopped = false;

	// Loads external bootrom binary for now
	LoadBootrom(bootrom_path);

	// Fake Nintendo Logo checksum
	// Temporary so the bootrom doesn't hang
	for(int i = 0; i < 0x30; i++)
	{
		Write8(0x104 + i, Read8(0xa8 + i));
	}
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
		if(ppu->Tick(cycles) == -1)
		{
			Stop();
		}
	}
	printf("\n\n\nExiting GameBoy...\n\n\n");
}

void GameBoy::Stop()
{
	Stopped = true;
}

void GameBoy::LoadBootrom(const char* bootrom_path)
{
	FILE* file = fopen(bootrom_path, "rb");
	
	uint8_t* buffer = new uint8_t[0x100];
	fread(buffer, 0x100, 1, file);

	GetMemory().GetRegionFromAddress(0x0000)->memcpy(0x0000, (void*) buffer, 0x100);

	delete[] buffer;
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

PPU& GameBoy::GetPPU()
{
	return *GetGameBoy()->ppu;
}

void GameBoy::SystemError(const char* error_msg, ...)
{
	va_list args;
	va_start(args, error_msg);
	printf("ERROR: "); vprintf(error_msg, args);
	va_end(args);
	instance->Stop();
}
