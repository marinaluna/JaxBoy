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

#include "Logger.h"

#include "../core/processor/Processor.h"
#include "../core/processor/Opcodes.h"
#include "../core/memory/MemoryMap.h"

#include <cstdio>
#include <iostream>
#include <iomanip>


namespace Debugger {

Logger::Logger(std::shared_ptr<Core::MemoryMap>& memory_map)
:
    memory_map (memory_map)
{}

void Logger::LogMessage(const std::string& msg)
{
    std::cout << msg << std::endl;
}

void Logger::LogError(const std::string& error_msg, bool fatal)
{
    // Set the color to red
    std::cout << "\033[31m\n";
    if(fatal)
    {
        std::cout << "FATAL: ";
    }
    else
    {
        std::cout << "WARN: ";
    }
    std::cout << error_msg;
    // and back to white again
    std::cout << "\033[0m\n";
}

void Logger::LogRegisters(const Core::Processor& processor)
{
    u8 zero = (processor.F_Zero)? 0b10000000 : 0;
    u8 subtract = (processor.F_Subtract)? 0b01000000 : 0;
    u8 halfcarry = (processor.F_HalfCarry)? 0b00100000 : 0;
    u8 carry = (processor.F_Carry)? 0b00010000 : 0;
    std::cout << "F: " << std::setw(2) << std::setfill('0') << std::hex << static_cast<int>(zero | subtract | halfcarry | carry) << "h\n";
    std::cout << "A: " << std::setw(2) << std::setfill('0') << std::hex << static_cast<int>(processor.reg_A) << "h\n";
    std::cout << "B: " << std::setw(2) << std::setfill('0') << std::hex << static_cast<int>(processor.reg_B) << "h\n";
    std::cout << "C: " << std::setw(2) << std::setfill('0') << std::hex << static_cast<int>(processor.reg_C) << "h\n";
    std::cout << "D: " << std::setw(2) << std::setfill('0') << std::hex << static_cast<int>(processor.reg_D) << "h\n";
    std::cout << "E: " << std::setw(2) << std::setfill('0') << std::hex << static_cast<int>(processor.reg_E) << "h\n";
    std::cout << "H: " << std::setw(2) << std::setfill('0') << std::hex << static_cast<int>(processor.reg_H) << "h\n";
    std::cout << "L: " << std::setw(2) << std::setfill('0') << std::hex << static_cast<int>(processor.reg_L) << "h\n";
    std::cout << "PC: " << std::setw(4) << std::setfill('0') << std::hex << processor.reg_PC.word << "h\n";
    std::cout << "SP: " << std::setw(4) << std::setfill('0') << std::hex << processor.reg_SP.word << "h\n";
    std::cout << std::endl;
}

void Logger::LogIORegisters()
{
    std::cout << "LCDC: " << std::setw(2) << std::setfill('0') << std::hex << static_cast<int>(memory_map->Read8(0xFF40)) << "h\n";
    std::cout << "STAT: " << std::setw(2) << std::setfill('0') << std::hex << static_cast<int>(memory_map->Read8(0xFF41)) << "h\n";
    std::cout << "SCY: " << std::setw(2) << std::setfill('0') << std::hex << static_cast<int>(memory_map->Read8(0xFF42)) << "h\n";
    std::cout << "SCX: " << std::setw(2) << std::setfill('0') << std::hex << static_cast<int>(memory_map->Read8(0xFF43)) << "h\n";
    std::cout << "LY: " << std::setw(2) << std::setfill('0') << std::hex << static_cast<int>(memory_map->Read8(0xFF44)) << "h\n";
    std::cout << "LYC: " << std::setw(2) << std::setfill('0') << std::hex << static_cast<int>(memory_map->Read8(0xFF45)) << "h\n";
    // TODO: the rest of them
    std::cout << std::endl;
}

void Logger::LogMemory(u16 address, u16 bytes)
{
    // print a column label at the beginning
    std::cout << "\033[33m" << "       00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F" << "\033[0m\n";

    for(int i = 0; i < bytes; i++)
    {
        // new line every 16 bytes
        // print the address of that line
        if((i % 16) == 0)
        {
            if(i != 0)
            {
                std::cout << "\n";
            }
            std::cout << "\033[33m" << std::setw(4) << std::setfill('0') << std::hex << address + i << "h: \033[0m";
        }
        std::cout << std::setw(2) << std::setfill('0') << std::hex << static_cast<int>(memory_map->Read8(address + i)) << " ";
    }
    std::cout << std::endl;
}

void Logger::LogVRAM()
{
    std::cout << "VRAM: 8000h - BFFFh\n";
    LogMemory(0x8000, 0x4000);
}

void Logger::LogDisassembly(u16 address, u16 instructions)
{
    u8 opcode;
    u8 operand8;
    u16 operand16;
    while(instructions-- > 0)
    {
        // print the address
        std::cout << "\033[33m" << std::setw(4) << std::setfill('0') << std::hex << address << "h: \033[0m";

        opcode = memory_map->Read8(address++);
        if(OPCODE_LOOKUP[opcode].length == 2)
        {
            // 2 byte instruction
            operand8 = memory_map->Read8(address++);
            printf(OPCODE_LOOKUP[opcode].name.c_str(), operand8);
        }
        else if(OPCODE_LOOKUP[opcode].length == 3)
        {
            // 3 byte instruction
            operand16 = memory_map->Read16(address++);
            printf(OPCODE_LOOKUP[opcode].name.c_str(), operand16);
        }
        else
        {
            // 1 byte instruction
            printf("%s", OPCODE_LOOKUP[opcode].name.c_str());
        }

        std::cout << std::endl;
    }
}


}; // namespace Debugger
