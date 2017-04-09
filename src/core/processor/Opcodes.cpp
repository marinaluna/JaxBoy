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

#include "../../common/Types.h"
#include "../../common/Globals.h"
#include "../memory/MemoryMap.h"


namespace Core {

// load
void Processor::ld_reg(Reg8& reg, u8 value)
{
    reg = value;
}
void Processor::ld_reg(Reg16& reg, u16 value)
{
    reg.word = value;
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
    F_Subtract = false;
    F_HalfCarry = (reg & 0x0F) + 1 > 0x0F;
    ++reg;

    F_Zero = reg == 0x00;
}
void Processor::inc(Reg16& reg16)
{
    ++reg16.word;
}

void Processor::dec(Reg8& reg)
{
    F_Subtract = true;
    F_HalfCarry = (reg & 0x0F) < 1;
    --reg;

    F_Zero = reg == 0x00;
}
void Processor::dec(Reg16& reg16)
{
    --reg16.word;
}

// add
void Processor::add(Reg8& reg, u8 value)
{
    F_Subtract = false;
    F_HalfCarry = ((reg & 0x0F) + (value & 0x0F)) > 0x0F;
    F_Carry = (reg + value) > 0xFF;
    reg += value;

    F_Zero = reg == 0x00;
}
void Processor::add(Reg16& reg, u16 value)
{
    F_Subtract = false;
    F_HalfCarry = ((reg.word & 0x0F00) + (value & 0x0F00)) > 0x0F00;
    F_Carry = (reg.word + value) > 0xFFFF;
    reg.word += value;
}
void Processor::add(Reg16& reg, s8 value)
{
    F_Subtract = false;
    F_Zero = false;
    // TODO: HalfCarry and Carry
    reg.word = reg.word + value;
}
void Processor::adc(Reg8& reg, u8 value)
{
    u8 adder = F_Carry? 1 : 0;
    F_Subtract = false;
    F_HalfCarry = ((reg & 0x0F) + (value & 0x0F) + adder) > 0x0F;
    F_Carry = (reg + value + adder > 0xFF);
    reg += value + adder;

    F_Zero = reg == 0x00;
}

// sub
void Processor::sub(Reg8& reg, u8 value)
{
    F_Subtract = true;
    F_HalfCarry = (reg & 0x0F) < (value & 0x0F);
    F_Carry = (reg < value);
    reg -= value;

    F_Zero = reg == 0x00;
}
void Processor::sbc(Reg8& reg, u8 value)
{
    u8 adder = F_Carry? 1 : 0;
    F_Subtract = true;
    F_HalfCarry = (reg & 0x0F) < ((value & 0x0F) + adder);
    F_Carry = reg < (value + adder);
    reg -= value - ((F_Carry)? 1 : 0);

    F_Zero = reg == 0x00;
}

// bitwise
void Processor::and8(Reg8& reg, u8 value)
{
    reg &= value;

    F_Subtract = false;
    F_HalfCarry = true;
    F_Carry = false;
    F_Zero = reg == 0x00;
}

void Processor::xor8(Reg8& reg, u8 value)
{
    reg ^= value;

    F_Subtract = false;
    F_HalfCarry = false;
    F_Carry = false;
    F_Zero = reg == 0x00;
}

void Processor::or8(Reg8& reg, u8 value)
{
    reg |= value;

    F_Subtract = false;
    F_HalfCarry = false;
    F_Carry = false;
    F_Zero = reg == 0x00;
}

// daa
void Processor::daa()
{
    if(F_Subtract)
    {
        if(F_Carry)
        {
            reg_A -= 0x60;
        }
        if(F_HalfCarry)
        {
            reg_A -= 0x06;
        }
    }
    else
    {
        if(F_Carry || reg_A > 0x99)
        {
            reg_A += 0x60;
            F_Carry = true;
        }
        if(F_HalfCarry || (reg_A & 0x0F) > 0x09)
        {
            reg_A += 0x06;
        }
    }
    F_Zero = reg_A == 0x00;
    F_HalfCarry = false;
}

// compare
void Processor::cp(u8 value)
{
    F_Subtract = true;
    F_HalfCarry = (reg_A & 0x0F) < (value & 0x0F);
    F_Carry = reg_A < value;
    F_Zero = (reg_A == value);
}

// jump
void Processor::jr(s8 amt)
{
    reg_PC.word += amt;
}

void Processor::jp(u16 addr)
{
    reg_PC.word = addr;
}

void Processor::call(u16 addr)
{
    push(reg_PC.word);
    reg_PC.word = addr;
}

void Processor::ret()
{
    pop(reg_PC);
}

// stack
void Processor::push(u16 value)
{
    reg_SP.word -= 2;
    memory_map->Write16(reg_SP.word, value);
}

void Processor::pop(Reg16& reg16)
{
    reg16.word = memory_map->Read16(reg_SP.word);
    reg_SP.word += 2;
}

// CB opcodes
// rotate
void Processor::rlc(Reg8& reg)
{
    bool newCarry = (reg & 0b10000000) != 0;
    reg <<= 1;
    reg |= (newCarry? 1 : 0);
    F_Carry = newCarry;
}
void Processor::rl(Reg8& reg)
{
    bool newCarry = (reg & 0b10000000) != 0;
    reg <<= 1;
    reg |= (F_Carry? 1 : 0);
    F_Carry = newCarry;
}
void Processor::rrc(Reg8& reg)
{
    bool newCarry = (reg & 0b00000001) != 0;
    reg >>= 1;
    reg |= (newCarry? 0b10000000 : 0);
    F_Carry = newCarry;
}
void Processor::rr(Reg8& reg)
{
    bool newCarry = (reg & 0b00000001) != 0;
    reg >>= 1;
    reg |= (F_Carry? 0b10000000 : 0);
    F_Carry = newCarry;
}

// shift
void Processor::sla(Reg8& reg)
{
    F_Carry = (reg & 0b10000000) != 0;
    reg <<= 1;
}
void Processor::srl(Reg8& reg)
{
    bool newCarry = (reg & 0b0000001) != 0;
    reg >>= 1;
    F_Carry = newCarry;
}

// bit
void Processor::bit(u8 byte, u8 bit)
{
    F_Zero = (byte & gBitMasks[bit]) == 0;
}

// swap
void Processor::swap(Reg8& reg)
{
    reg = ((reg & 0xFF) << 8) | ((reg & 0xFF00) >> 8);

    F_Zero = reg == 0;
}

// reset bit
void Processor::res(Reg8& reg, u8 bit)
{
    reg &= ~gBitMasks[bit];
}
void Processor::res_addr(u16 addr, u8 bit)
{
    memory_map->Write8(addr, memory_map->Read8(addr) & ~gBitMasks[bit]);
}

// set bit
void Processor::set(Reg8& reg, u8 bit)
{
    reg |= gBitMasks[bit];
}

}; // namespace Core
