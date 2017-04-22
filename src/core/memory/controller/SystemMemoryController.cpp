// Copyright (C) 2017 Ryan Terry
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "SystemMemoryController.h"
#include "../../GameBoy.h"

#include <string>


namespace Memory {

SystemMemoryController::SystemMemoryController(Core::GameBoy* gameboy)
: MemoryController (gameboy),

  PageROM (new MemoryPage(0x0000, 0x8000)),
  PageVRAM (new MemoryPage(0x8000, 0x2000)),
  PageSRAM (new MemoryPage(0xA000, 0x2000)),
  PageWRAM (new MemoryPage(0xC000, 0x2000)),
  PageOAM (new MemoryPage(0xFE00, 0x00A0)),
  PageHighRAM (new MemoryPage(0xFF80, 0x007F))
{}

std::unique_ptr<MemoryPage>& SystemMemoryController::GetPage(u16 address)
{
    if(address >= 0x0000 && address <= 0x7FFF)
        return PageROM;
    if(address >= 0x8000 && address <= 0x9FFF)
        return PageVRAM;
    if(address >= 0xA000 && address <= 0xBFFF)
        return PageSRAM;
    if(address >= 0xC000 && address <= 0xDFFF)
        return PageWRAM;
    if(address >= 0xFE00 && address <= 0xFE9F)
        return PageOAM;
    if(address >= 0xFF80 && address <= 0xFFFE)
        return PageHighRAM;

    throw std::out_of_range("Address out of bounds!");
}

void SystemMemoryController::Write8(u16 address, u8 data)
{
    try
    {
        std::unique_ptr<MemoryPage>& page = GetPage(address);
        address -= page->GetBase();
        page->GetBytes().at(address) = data;
    }
    catch(std::out_of_range& e)
    {
        gameboy->SystemError("SystemMemoryController: " + std::string(e.what()));
    }
}

void SystemMemoryController::Write16(u16 address, u16 data)
{
    try
    {
        std::unique_ptr<MemoryPage>& page = GetPage(address);
        address -= page->GetBase();
        page->GetBytes().at(address) = data & 0x00FF;
        page->GetBytes().at(address + 1) = (data & 0xFF00) >> 8;
    }
    catch(std::out_of_range& e)
    {
        gameboy->SystemError("SystemMemoryController: " + std::string(e.what()));
    }
}

u8 SystemMemoryController::Read8(u16 address)
{
    try
    {
        std::unique_ptr<MemoryPage>& page = GetPage(address);
        address -= page->GetBase();
        return page->GetBytes().at(address);
    }
    catch(std::out_of_range& e)
    {
        gameboy->SystemError("SystemMemoryController: " + std::string(e.what()));
        return 0xFF;
    }
}

u16 SystemMemoryController::Read16(u16 address)
{
    try
    {
        std::unique_ptr<MemoryPage>& page = GetPage(address);
        address -= page->GetBase();
        return page->GetBytes().at(address) | (page->GetBytes().at(address + 1) << 8);
    }
    catch(std::out_of_range& e)
    {
        gameboy->SystemError("SystemMemoryController: " + std::string(e.what()));
        return 0xFFFF;
    }
}

void SystemMemoryController::WriteBytes(const u8* src, u16 destination, u16 size)
{
    try
    {
        // TODO: won't work across page boundaries
        // because the code is slow when used
        std::unique_ptr<MemoryPage>& page = GetPage(destination);
        std::memcpy(page->GetRaw() + (destination - page->GetBase()), src, size);
    //  int counter = 0;
    //  while(counter < size)
    //  {
    //      std::unique_ptr<MemoryPage>& page = GetPage(destination);
    //      page->GetBytes().at(destination - page->GetBase()) = src.at(counter++);
    //      destination++;
    //  }
    }
    catch(std::out_of_range& e)
    {
        gameboy->SystemError("SystemMemoryController: " + std::string(e.what()));
    }
}

void SystemMemoryController::ReadBytes(u8* destination, u16 src, u16 size)
{
    try
    {
        std::unique_ptr<MemoryPage>& page = GetPage(src);
        std::memcpy(destination, page->GetRaw() + (src - page->GetBase()), size);
    //  int counter = 0;
    //  while(counter < size)
    //  {
    //      std::unique_ptr<MemoryPage>& page = GetPage(src);
    //      destination[(counter++)] = page->GetBytes().at(src - page->GetBase());
    //      src++;
    //  }
    }
    catch(std::out_of_range& e)
    {
        gameboy->SystemError("SystemMemoryController: " + std::string(e.what()));
    }
}

}; // namespace Memory
