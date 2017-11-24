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

#pragma once

#include "../../common/Types.h"

#include <memory>


namespace Memory {
    class MemoryBus;
}; // namespace Memory

// needed to make LogRegisters a friend of Processor
namespace Core {
    class Processor;
}; // namespace Core
namespace Debug {
    namespace Logger {
        void LogRegisters(const Core::Processor& processor);
    }; // namespace Logger
}; // namespace Debug

namespace Core {
class GameBoy;

class Processor
{
    friend class Memory::MemoryBus;
    friend void Debug::Logger::LogRegisters(const Core::Processor& processor);

    // 16-bit program counter and stack pointer
    Reg16 reg_PC;
    Reg16 reg_SP;
    // 8-bit register pairs are linked
    // together to make 16-bit registers
    Reg16 reg_AF;
    Reg16 reg_BC;
    Reg16 reg_DE;
    Reg16 reg_HL;
    // 8-bit registers point to halves of 16-bit pairs
    Reg8& reg_B = reg_BC.high;
    Reg8& reg_C = reg_BC.low;
    Reg8& reg_D = reg_DE.high;
    Reg8& reg_E = reg_DE.low;
    Reg8& reg_H = reg_HL.high;
    Reg8& reg_L = reg_HL.low;
    Reg8& reg_A = reg_AF.high;
    // flags
    bool F_Zero;
    bool F_Subtract;
    bool F_HalfCarry;
    bool F_Carry;
    // Interrupt registers
    bool MasterInterruptsEnabled;
    u8 InterruptsEnabled;
    u8 InterruptsRequested;
    int TickInterrupts();

    GameBoy* gameboy;
    std::shared_ptr<Memory::MemoryBus> memory_bus;

public:
    Processor(GameBoy* gameboy,
              std::shared_ptr<Memory::MemoryBus>& memory_bus);

    int Tick();

    void StartDMATransfer(u8 addrH);

    // fetches operand and increments PC
    u8 GetOperand8();
    u16 GetOperand16();

    int ExecuteNext();
    u8 ExecuteCBOpcode();

    // Instructions
    // load
    void ld(Reg8& reg, u8 value);
    void ld(Reg16& reg, u16 value);
    void ldAt(u16 addr, u8 value);
    void ldAt(u16 addr, u16 value);
    // inc/dec
    void inc(Reg8& reg);
    void inc(Reg16& reg16);
    void incAt(u16 addr);
    void dec(Reg8& reg);
    void dec(Reg16& reg16);
    void decAt(u16 addr);
    // add
    void add(Reg8& reg, u8 value);
    void add(Reg16& reg, u16 value);
    void add(Reg16& reg, s8 value);
    void adc(Reg8& reg, u8 value);
    // sub
    void sub(Reg8& reg, u8 value);
    void sbc(Reg8& reg, u8 value);
    // bitwise
    void and8(Reg8& reg, u8 value);
    void xor8(Reg8& reg, u8 value);
    void or8(Reg8& reg, u8 value);
    // daa
    void daa();
    // compare
    void cp(u8 value);
    // jump
    void jr(s8 amt);
    void jp(u16 addr);
    void call(u16 addr);
    void ret();
    // stack
    void push(u16 value);
    void pop(Reg16& reg16);

    // CB opcodes
    // rotate
    void rlc(Reg8& reg, bool zero);
    void rlcAt(u16 addr, bool zero);
    void rl(Reg8& reg, bool zero);
    void rlAt(u16 addr, bool zero);
    void rrc(Reg8& reg, bool zero);
    void rrcAt(u16 addr, bool zero);
    void rr(Reg8& reg, bool zero);
    void rrAt(u16 addr, bool zero);
    // shift
    void sla(Reg8& reg);
    void slaAt(u16 addr);
    void srl(Reg8& reg);
    void srlAt(u16 addr);
    void sra(Reg8& reg);
    void sraAt(u16 addr);
    // bit
    void bit(u8 byte, u8 bit);
    // swap
    void swap(Reg8& reg);
    void swapAt(u16 addr);
    // cpl
    void cpl(Reg8& reg);
    void ccf();
    // set carry
    void scf();
    // reset bit
    void res(Reg8& reg, u8 bit);
    void resAt(u16 addr, u8 bit);
    // set bit
    void set(Reg8& reg, u8 bit);
    void setAt(u16 addr, u8 bit);
};

}; // namespace Core
