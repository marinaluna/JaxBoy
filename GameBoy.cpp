#include "Rom.h"
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
    IsDebugMode = false;

    instance = this;

    processor = new Processor();
    ppu = new PPU();
    memory_map = new MemoryMap();

    Stopped = false;

    LoadRom(rom_name);
    // Bootrom LLE rather than HLE for now
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
        if(ppu->Tick(cycles) == -1)
        {
            Stop();
        }
    }
    printf("\n\n\nExiting JaxBoy...\n\n\n");
}

void GameBoy::Stop()
{
    Stopped = true;
}

void GameBoy::LoadRom(const char* rom_name)
{
    FILE* file = fopen(rom_name, "rb");
    
    uint8_t* buffer = new uint8_t[0x8000];
    fread(buffer, 0x8000, 1, file);

    GetMemory().GetRegionFromAddress(0x0000)->memcpy(0x0000, (void*) buffer, 0x8000);

    loaded_rom = new Rom(rom_name, buffer);

    delete[] buffer;
    fclose(file);
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

void GameBoy::ReplaceInterruptVectors()
{
    GetMemory().GetRegionFromAddress(0x0000)->memcpy(0x0000, (void*) loaded_rom->GetInterruptVectorsPtr(), 0x100);
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
