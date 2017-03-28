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

#include "Processor.h"

#include "../memory/MemoryMap.h"

#include "../../common/Globals.h"
#include "../../common/Types.h"


namespace Core {

// load
void Processor::ld_reg(Reg8& reg, u8 value)
{
    reg = value;
}
void Processor::ld_reg(Reg16& reg, u16 value)
{
    reg = value;
}

// write
void Processor::ld_addr(u16 addr, u8 value)
{
    memory_map->Write8(addr, value);
}
void Processor::ld_addr(u16 addr, u16 value)
{
    memory_map->Write16(addr, value);
}

// inc/dec
void Processor::inc(Reg8& reg)
{
    ++reg;
}
void Processor::inc(Reg16& reg16)
{
    if(++reg16.low == 0x00)
        ++reg16.high;
}

void Processor::dec(Reg8& reg)
{
    --reg;

    F_Zero = reg == 0x00;
}
void Processor::dec(Reg16& reg16)
{
    if(--reg16.low == 0xFF)
        --reg16.high;

    F_Zero = reg16 == 0x0000;
}

// add
void Processor::add(Reg8& reg, u8 value)
{
    reg += value;
}
void Processor::add(Reg16& reg, u16 value)
{
    reg += value;
}
void Processor::add(Reg16& reg, s8 value)
{
    reg = static_cast<u16>(reg) + value;
}
void Processor::adc(Reg8& reg, u8 value)
{
    reg += value + ((F_Carry)? 1 : 0);
}

// sub
void Processor::sub(Reg8& reg, u8 value)
{
    reg -= value;
}
void Processor::sbc(Reg8& reg, u8 value)
{
    reg -= value - ((F_Carry)? 1 : 0);
}

// bitwise
void Processor::and8(Reg8& reg, u8 value)
{
    reg &= value;
}

void Processor::xor8(Reg8& reg, u8 value)
{
    reg ^= value;
}

void Processor::or8(Reg8& reg, u8 value)
{
    reg |= value;
}

// compare
void Processor::cp(u8 value)
{
    F_Zero = (reg_A == value);
}

// jump
void Processor::jr(s8 amt)
{
    reg_PC += amt;
}

void Processor::jp(u16 addr)
{
    reg_PC = addr;
}

void Processor::call(u16 addr)
{
    push(reg_PC + 2);
    reg_PC = addr;
}

void Processor::ret()
{
    pop(reg_PC);
}

// stack
void Processor::push(u16 value)
{
    reg_SP -= 2;
    memory_map->Write16(reg_SP, value);
}

void Processor::pop(Reg16& reg16)
{
    reg16 = memory_map->Read16(reg_SP);
    reg_SP += 2;
}

// CB opcodes
// rotate
void Processor::rl(Reg8& reg)
{
    bool newCarry = (reg & 0b10000000) != 0;
    reg <<= 1;
    reg |= (F_Carry? 1 : 0);
    F_Carry = newCarry;
}

// bit
void Processor::bit(Reg8& reg, u8 bit)
{
    F_Zero = (reg & gBitMasks[bit]) == 0;
}

}; // namespace Core
