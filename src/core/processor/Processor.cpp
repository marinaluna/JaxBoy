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
#include "Opcodes.h"

#include "../GameBoy.h"
#include "../memory/MemoryMap.h"

#include "../../debug/Logger.h"

#include <memory>

using namespace Debug;


namespace Core {

Processor::Processor(GameBoy* gameboy, std::shared_ptr<MemoryMap>& memory_map, std::shared_ptr<Debug::Logger>& logger)
:
    gameboy (gameboy),
    memory_map (memory_map),
    logger (logger)
{
    reg_PC.word = 0x0000;
    reg_SP.word = 0x0000;
    reg_A = reg_B = reg_C = reg_D = reg_E = reg_H = reg_L = 0x00;

    MasterInterruptsEnabled = true;
}

int Processor::Tick()
{
    int new_cycles = ExecuteNext();
    new_cycles += TickInterrupts();

    return new_cycles;
}

int Processor::TickInterrupts()
{
    if(MasterInterruptsEnabled)
    {
        // Check if there is any interrupts we can execute
        if(InterruptsEnabled != 0 && InterruptsRequested != 0)
        {
            // Mask out only the interrupts that have
            // both IE and IF set
            u8 interruptsPending = InterruptsEnabled & InterruptsRequested;
            // The priority of each interrupt is determined
            // by their position in the bit mask of IE/IF
            //
            // 00000001b - V-Blank: highest priority
            // 00000010b - STAT
            // 00000100b - Timer
            // 00001000b - Serial
            // 00010000b - JoyPad: lowest priority
            if((interruptsPending & 0b00000001) != 0)
            {
                // V-Blank
                MasterInterruptsEnabled = false;
                InterruptsRequested &= ~0b00000001;

                call(0x0040);
                return 12;
            }
            if((interruptsPending & 0b00000010) != 0)
            {
                // STAT
                MasterInterruptsEnabled = false;
                InterruptsRequested &= ~0b00000010;

                call(0x0048);
                return 12;
            }
            if((interruptsPending & 0b00000100) != 0)
            {
                // Timer
                MasterInterruptsEnabled = false;
                InterruptsRequested &= ~0b00000100;

                call(0x0050);
                return 12;
            }
            if((interruptsPending & 0b00001000) != 0)
            {
                // Serial
                MasterInterruptsEnabled = false;
                InterruptsRequested &= ~0b00001000;

                call(0x0058);
                return 12;
            }
            if((interruptsPending & 0b00010000) != 0)
            {
                // JoyPad
                MasterInterruptsEnabled = false;
                InterruptsRequested &= ~0b00010000;

                call(0x0060);
                return 12;
            }
        }
    }
    
    return 0;
}

// fetches operand and increments PC
// TODO: inline these
u8 Processor::GetOperand8()
{
    return memory_map->Read8(reg_PC.word++);
}

u16 Processor::GetOperand16()
{
    u16 operand = memory_map->Read16(reg_PC.word);
    reg_PC.word += 2;
    return operand;
}

// Decodes and executes instruction
int Processor::ExecuteNext()
{
    u8 opcode = memory_map->Read8(reg_PC.word++);
    bool branch_taken = false;
    // the table to look for opcode information in
    const Opcode* opcode_lookup_table = OPCODE_LOOKUP;

    switch(opcode)
    {
        // CB
        case 0xCB:
            opcode_lookup_table = CB_OPCODE_LOOKUP;
            opcode = ExecuteCBOpcode();
            break;
        // NOP
        case 0x00:
            break;
        // STOP
        case 0x10:
            break;
        // HALT
        case 0x76:
            break;
        // DI
        case 0xF3:
            MasterInterruptsEnabled = false; break;
        // EI
        case 0xFB:
            MasterInterruptsEnabled = true; break;
        
        // LD reg8, u8
        case 0x06:
            ld_reg(reg_B, GetOperand8()); break;
        case 0x0E:
            ld_reg(reg_C, GetOperand8()); break;
        case 0x16:
            ld_reg(reg_D, GetOperand8()); break;
        case 0x1E:
            ld_reg(reg_E, GetOperand8()); break;
        case 0x26:
            ld_reg(reg_H, GetOperand8()); break;
        case 0x2E:
            ld_reg(reg_L, GetOperand8()); break;
        case 0x3E:
            ld_reg(reg_A, GetOperand8()); break;
        case 0x0A:
            ld_reg(reg_A, memory_map->Read8(reg_BC.word)); break;
        case 0x1A:
            ld_reg(reg_A, memory_map->Read8(reg_DE.word)); break;
        case 0x2A:
            ld_reg(reg_A, memory_map->Read8(reg_HL.word++)); break;
        case 0x3A:
            ld_reg(reg_A, memory_map->Read8(reg_HL.word--)); break;
        case 0x46:
            ld_reg(reg_B, memory_map->Read8(reg_HL.word)); break;
        case 0x4E:
            ld_reg(reg_C, memory_map->Read8(reg_HL.word)); break;
        case 0x56:
            ld_reg(reg_D, memory_map->Read8(reg_HL.word)); break;
        case 0x5E:
            ld_reg(reg_E, memory_map->Read8(reg_HL.word)); break;
        case 0x66:
            ld_reg(reg_H, memory_map->Read8(reg_HL.word)); break;
        case 0x6E:
            ld_reg(reg_L, memory_map->Read8(reg_HL.word)); break;
        case 0x7E:
            ld_reg(reg_A, memory_map->Read8(reg_HL.word)); break;
        case 0xF0:
            ld_reg(reg_A, memory_map->Read8(0xFF00 + GetOperand8())); break;
        case 0xF2:
            ld_reg(reg_A, memory_map->Read8(0xFF00 + reg_C)); break;
        case 0xFA:
            ld_reg(reg_A, memory_map->Read8(GetOperand16())); break;
        case 0x40:
            ld_reg(reg_B, reg_B); break;
        case 0x41:
            ld_reg(reg_B, reg_C); break;
        case 0x42:
            ld_reg(reg_B, reg_D); break;
        case 0x43:
            ld_reg(reg_B, reg_E); break;
        case 0x44:
            ld_reg(reg_B, reg_H); break;
        case 0x45:
            ld_reg(reg_B, reg_L); break;
        case 0x47:
            ld_reg(reg_B, reg_A); break;
        case 0x48:
            ld_reg(reg_C, reg_B); break;
        case 0x49:
            ld_reg(reg_C, reg_C); break;
        case 0x4A:
            ld_reg(reg_C, reg_D); break;
        case 0x4B:
            ld_reg(reg_C, reg_E); break;
        case 0x4C:
            ld_reg(reg_C, reg_H); break;
        case 0x4D:
            ld_reg(reg_C, reg_L); break;
        case 0x4F:
            ld_reg(reg_C, reg_A); break;
        case 0x50:
            ld_reg(reg_D, reg_B); break;
        case 0x51:
            ld_reg(reg_D, reg_C); break;
        case 0x52:
            ld_reg(reg_D, reg_D); break;
        case 0x53:
            ld_reg(reg_D, reg_E); break;
        case 0x54:
            ld_reg(reg_D, reg_H); break;
        case 0x55:
            ld_reg(reg_D, reg_L); break;
        case 0x57:
            ld_reg(reg_D, reg_A); break;
        case 0x58:
            ld_reg(reg_E, reg_B); break;
        case 0x59:
            ld_reg(reg_E, reg_C); break;
        case 0x5A:
            ld_reg(reg_E, reg_D); break;
        case 0x5B:
            ld_reg(reg_E, reg_E); break;
        case 0x5C:
            ld_reg(reg_E, reg_H); break;
        case 0x5D:
            ld_reg(reg_E, reg_L); break;
        case 0x5F:
            ld_reg(reg_E, reg_A); break;
        case 0x60:
            ld_reg(reg_H, reg_B); break;
        case 0x61:
            ld_reg(reg_H, reg_C); break;
        case 0x62:
            ld_reg(reg_H, reg_D); break;
        case 0x63:
            ld_reg(reg_H, reg_E); break;
        case 0x64:
            ld_reg(reg_H, reg_H); break;
        case 0x65:
            ld_reg(reg_H, reg_L); break;
        case 0x67:
            ld_reg(reg_H, reg_A); break;
        case 0x68:
            ld_reg(reg_L, reg_B); break;
        case 0x69:
            ld_reg(reg_L, reg_C); break;
        case 0x6A:
            ld_reg(reg_L, reg_D); break;
        case 0x6B:
            ld_reg(reg_L, reg_E); break;
        case 0x6C:
            ld_reg(reg_L, reg_H); break;
        case 0x6D:
            ld_reg(reg_L, reg_L); break;
        case 0x6F:
            ld_reg(reg_L, reg_A); break;
        case 0x78:
            ld_reg(reg_A, reg_B); break;
        case 0x79:
            ld_reg(reg_A, reg_C); break;
        case 0x7A:
            ld_reg(reg_A, reg_D); break;
        case 0x7B:
            ld_reg(reg_A, reg_E); break;
        case 0x7C:
            ld_reg(reg_A, reg_H); break;
        case 0x7D:
            ld_reg(reg_A, reg_L); break;
        case 0x7F:
            ld_reg(reg_A, reg_A); break;
        // LD reg16, u16
        case 0x01:
            ld_reg(reg_BC, GetOperand16()); break;
        case 0x11:
            ld_reg(reg_DE, GetOperand16()); break;
        case 0x21:
            ld_reg(reg_HL, GetOperand16()); break;
        case 0x31:
            ld_reg(reg_SP, GetOperand16()); break;
        case 0xF8:
            ld_reg(reg_HL, reg_SP.word + static_cast<s8>(GetOperand8())); break;

        // LD (addr), u8
        case 0x02:
            ld_addr(reg_BC.word, reg_A); break;
        case 0x12:
            ld_addr(reg_DE.word, reg_A); break;
        case 0x22:
            ld_addr(reg_HL.word++, reg_A); break;
        case 0x32:
            ld_addr(reg_HL.word--, reg_A); break;
        case 0x36:
            ld_addr(reg_HL.word, GetOperand8()); break;
        case 0x70:
            ld_addr(reg_HL.word, reg_B); break;
        case 0x71:
            ld_addr(reg_HL.word, reg_C); break;
        case 0x72:
            ld_addr(reg_HL.word, reg_D); break;
        case 0x73:
            ld_addr(reg_HL.word, reg_E); break;
        case 0x74:
            ld_addr(reg_HL.word, reg_H); break;
        case 0x75:
            ld_addr(reg_HL.word, reg_L); break;
        case 0x77:
            ld_addr(reg_HL.word, reg_A); break;
        case 0xE0:
            ld_addr(0xFF00 + GetOperand8(), reg_A); break;
        case 0xE2:
            ld_addr(0xFF00 + reg_C, reg_A); break;
        case 0xEA:
            ld_addr(GetOperand16(), reg_A); break;
        // LD (addr), u16
        case 0x08:
            ld_addr(GetOperand16(), reg_SP.word); break;

        // INC reg8
        case 0x04:
            inc(reg_B); break;
        case 0x0C:
            inc(reg_C); break;
        case 0x14:
            inc(reg_D); break;
        case 0x1C:
            inc(reg_E); break;
        case 0x24:
            inc(reg_H); break;
        case 0x2C:
            inc(reg_L); break;
        case 0x3C:
            inc(reg_A); break;
        // INC reg16
        case 0x03:
            inc(reg_BC); break;
        case 0x13:
            inc(reg_DE); break;
        case 0x23:
            inc(reg_HL); break;
        case 0x33:
            inc(reg_SP); break;
        // INC (HL)
        case 0x34:
            memory_map->Write8(reg_HL.word, memory_map->Read8(reg_HL.word) + 1); break;

        // DEC reg8
        case 0x05:
            dec(reg_B); break;
        case 0x0D:
            dec(reg_C); break;
        case 0x15:
            dec(reg_D); break;
        case 0x1D:
            dec(reg_E); break;
        case 0x25:
            dec(reg_H); break;
        case 0x2D:
            dec(reg_L); break;
        case 0x3D:
            dec(reg_A); break;
        // DEC reg16
        case 0x0B:
            dec(reg_BC); break;
        case 0x1B:
            dec(reg_DE); break;
        case 0x2B:
            dec(reg_HL); break;
        case 0x3B:
            dec(reg_SP); break;
        // DEC (HL)
        case 0x35:
            memory_map->Write8(reg_HL.word, memory_map->Read8(reg_HL.word) - 1); break;

        // ADD reg8, u8
        case 0x80:
            add(reg_A, reg_B); break;
        case 0x81:
            add(reg_A, reg_C); break;
        case 0x82:
            add(reg_A, reg_D); break;
        case 0x83:
            add(reg_A, reg_E); break;
        case 0x84:
            add(reg_A, reg_H); break;
        case 0x85:
            add(reg_A, reg_L); break;
        case 0x86:
            add(reg_A, memory_map->Read8(reg_HL.word)); break;
        case 0x87:
            add(reg_A, reg_A); break;
        case 0xC6:
            add(reg_A, GetOperand8()); break;
        // ADD reg16, u16
        case 0x09:
            add(reg_HL, reg_BC.word); break;
        case 0x19:
            add(reg_HL, reg_DE.word); break;
        case 0x29:
            add(reg_HL, reg_HL.word); break;
        case 0x39:
            add(reg_HL, reg_SP.word); break;
        // ADD reg16, s8
        case 0xE8:
            add(reg_SP, static_cast<s8>(GetOperand8())); break;

        // ADC reg8, u8
        case 0x88:
            adc(reg_A, reg_B); break;
        case 0x89:
            adc(reg_A, reg_C); break;
        case 0x8A:
            adc(reg_A, reg_D); break;
        case 0x8B:
            adc(reg_A, reg_E); break;
        case 0x8C:
            adc(reg_A, reg_H); break;
        case 0x8D:
            adc(reg_A, reg_L); break;
        case 0x8E:
            adc(reg_A, memory_map->Read8(GetOperand8())); break;
        case 0x8F:
            adc(reg_A, reg_A); break;
        case 0xCE:
            adc(reg_A, GetOperand8()); break;

        // SUB reg8, u8
        case 0x90:
            sub(reg_A, reg_B); break;
        case 0x91:
            sub(reg_A, reg_C); break;
        case 0x92:
            sub(reg_A, reg_D); break;
        case 0x93:
            sub(reg_A, reg_E); break;
        case 0x94:
            sub(reg_A, reg_H); break;
        case 0x95:
            sub(reg_A, reg_L); break;
        case 0x96:
            sub(reg_A, memory_map->Read8(GetOperand8())); break;
        case 0x97:
            sub(reg_A, reg_A); break;
        case 0xD6:
            sub(reg_A, GetOperand8()); break;

        // SBC reg8, u8
        case 0x98:
            sbc(reg_A, reg_B); break;
        case 0x99:
            sbc(reg_A, reg_C); break;
        case 0x9A:
            sbc(reg_A, reg_D); break;
        case 0x9B:
            sbc(reg_A, reg_E); break;
        case 0x9C:
            sbc(reg_A, reg_H); break;
        case 0x9D:
            sbc(reg_A, reg_L); break;
        case 0x9E:
            sbc(reg_A, memory_map->Read8(GetOperand8())); break;
        case 0x9F:
            sbc(reg_A, reg_A); break;
        case 0xDE:
            sbc(reg_A, GetOperand8()); break;

        // AND reg8, u8
        case 0xA0:
            and8(reg_A, reg_B); break;
        case 0xA1:
            and8(reg_A, reg_C); break;
        case 0xA2:
            and8(reg_A, reg_D); break;
        case 0xA3:
            and8(reg_A, reg_E); break;
        case 0xA4:
            and8(reg_A, reg_H); break;
        case 0xA5:
            and8(reg_A, reg_L); break;
        case 0xA6:
            and8(reg_A, memory_map->Read8(GetOperand8())); break;
        case 0xA7:
            and8(reg_A, reg_A); break;
        case 0xE6:
            and8(reg_A, GetOperand8()); break;

        // XOR reg8, u8
        case 0xA8:
            xor8(reg_A, reg_B); break;
        case 0xA9:
            xor8(reg_A, reg_C); break;
        case 0xAA:
            xor8(reg_A, reg_D); break;
        case 0xAB:
            xor8(reg_A, reg_E); break;
        case 0xAC:
            xor8(reg_A, reg_H); break;
        case 0xAD:
            xor8(reg_A, reg_L); break;
        case 0xAE:
            xor8(reg_A, memory_map->Read8(GetOperand8())); break;
        case 0xAF:
            xor8(reg_A, reg_A); break;
        case 0xEE:
            xor8(reg_A, GetOperand8()); break;
        case 0x2F:
            xor8(reg_A, 0xFF); break;

        // OR reg8, u8
        case 0xB0:
            or8(reg_A, reg_B); break;
        case 0xB1:
            or8(reg_A, reg_C); break;
        case 0xB2:
            or8(reg_A, reg_D); break;
        case 0xB3:
            or8(reg_A, reg_E); break;
        case 0xB4:
            or8(reg_A, reg_H); break;
        case 0xB5:
            or8(reg_A, reg_L); break;
        case 0xB6:
            or8(reg_A, memory_map->Read8(GetOperand8())); break;
        case 0xB7:
            or8(reg_A, reg_A); break;
        case 0xF6:
            or8(reg_A, GetOperand8()); break;

        // RLC reg8
        case 0x07:
            rlc(reg_A); break;
        // RL reg8
        case 0x17:
            rl(reg_A); break;
        // RRC reg8
        case 0x0F:
            rrc(reg_A); break;

        // DAA
        case 0x27:
            // TODO
            break;

        // CP u8
        case 0xB8:
            cp(reg_B); break;
        case 0xB9:
            cp(reg_C); break;
        case 0xBA:
            cp(reg_D); break;
        case 0xBB:
            cp(reg_E); break;
        case 0xBC:
            cp(reg_H); break;
        case 0xBD:
            cp(reg_L); break;
        case 0xBE:
            cp(memory_map->Read8(reg_HL.word)); break;
        case 0xBF:
            cp(reg_A); break;
        case 0xFE:
            cp(GetOperand8()); break;

        // JR s8
        case 0x18:
            jr(static_cast<s8>(GetOperand8()));
            break;
        case 0x20:
            if(!F_Zero) {
                branch_taken = true;
                jr(static_cast<s8>(GetOperand8()));
                break;
            }
            reg_PC.word++;
            break;
        case 0x28:
            if(F_Zero) {
                branch_taken = true;
                jr(static_cast<s8>(GetOperand8()));
                break;
            }
            reg_PC.word++;
            break;
        case 0x30:
            if(!F_Carry) {
                branch_taken = true;
                jr(static_cast<s8>(GetOperand8()));
                break;
            }
            reg_PC.word++;
            break;
        case 0x38:
            if(F_Carry) {
                branch_taken = true;
                jr(static_cast<s8>(GetOperand8()));
                break;
            }
            reg_PC.word++;
            break;

        // JP u16
        case 0xC3:
            jp(GetOperand16());
            break;
        case 0xC2:
            if(!F_Zero) {
                branch_taken = true;
                jp(GetOperand16());
                break;
            }
            reg_PC.word += 2;
            break;
        case 0xCA:
            if(F_Zero) {
                branch_taken = true;
                jp(GetOperand16());
                break;
            }
            reg_PC.word += 2;
            break;
        case 0xD2:
            if(!F_Carry) {
                branch_taken = true;
                jp(GetOperand16());
                break;
            }
            reg_PC.word += 2;
            break;
        case 0xDA:
            if(F_Carry) {
                branch_taken = true;
                jp(GetOperand16());
                break;
            }
            reg_PC.word += 2;
            break;
        case 0xE9:
            jp(reg_HL.word);
            break;

        // CALL u16
        case 0xCD:
            call(GetOperand16());
            break;
        case 0xC4:
            if(!F_Zero) {
                branch_taken = true;
                call(GetOperand16());
                break;
            }
            reg_PC.word += 2;
            break;
        case 0xCC:
            if(F_Zero) {
                branch_taken = true;
                call(GetOperand16());
                break;
            }
            reg_PC.word += 2;
            break;
        case 0xD4:
            if(!F_Carry) {
                branch_taken = true;
                call(GetOperand16());
                break;
            }
            reg_PC.word += 2;
            break;
        case 0xDC:
            if(F_Carry) {
                branch_taken = true;
                call(GetOperand16());
                break;
            }
            reg_PC.word += 2;
            break;

        // RST XXh
        case 0xC7:
            call(0x0000); break;
        case 0xCF:
            call(0x0008); break;
        case 0xD7:
            call(0x0010); break;
        case 0xDF:
            call(0x0018); break;
        case 0xE7:
            call(0x0020); break;
        case 0xEF:
            call(0x0028); break;
        case 0xF7:
            call(0x0030); break;
        case 0xFF:
            call(0x0038); break;

        // RET
        case 0xC9:
            ret();
            break;
        case 0xC0:
            if(!F_Zero) {
                branch_taken = true;
                ret();
                break;
            }
            break;
        case 0xC8:
            if(F_Zero) {
                branch_taken = true;
                ret();
                break;
            }
            break;
        case 0xD0:
            if(!F_Carry) {
                branch_taken = true;
                ret();
                break;
            }
            break;
        case 0xD8:
            if(F_Carry) {
                branch_taken = true;
                ret();
                break;
            }
            break;
        case 0xD9:
            MasterInterruptsEnabled = true;
            ret();
            break;

        // PUSH reg16
        case 0xC5:
            push(reg_BC.word); break;
        case 0xD5:
            push(reg_DE.word); break;
        case 0xE5:
            push(reg_HL.word); break;
        case 0xF5:
            push(reg_AF.word); break;

        // POP reg16
        case 0xC1:
            pop(reg_BC); break;
        case 0xD1:
            pop(reg_DE); break;
        case 0xE1:
            pop(reg_HL); break;
        case 0xF1:
            pop(reg_AF); break;

        default:
            logger->LogDisassembly(reg_PC.word - 1, 1);
            logger->Log(LogType::FATAL, "Unknown opcode!");
            gameboy->Stop();
    }

    return (!branch_taken)?
        opcode_lookup_table[opcode].cycles : opcode_lookup_table[opcode].cycles_branch;
}

u8 Processor::ExecuteCBOpcode()
{
    u8 opcode = memory_map->Read8(reg_PC.word++);

    switch(opcode)
    {
        // RL reg8
        case 0x11:
            rl(reg_C); break;

        // SLA reg8
        case 0x27:
            sla(reg_A); break;

        // SWAP reg8
        case 0x37:
            swap(reg_A); break;

        // SRL reg8
        case 0x3F:
            srl(reg_A); break;

        // BIT x, u8
        case 0x40:
            bit(reg_B, 0); break;
        case 0x41:
            bit(reg_C, 0); break;
        case 0x42:
            bit(reg_D, 0); break;
        case 0x43:
            bit(reg_E, 0); break;
        case 0x44:
            bit(reg_H, 0); break;
        case 0x45:
            bit(reg_L, 0); break;
        case 0x46:
            bit(memory_map->Read8(reg_HL.word), 0); break;
        case 0x47:
            bit(reg_A, 0); break;
        case 0x48:
            bit(reg_B, 1); break;
        case 0x49:
            bit(reg_C, 1); break;
        case 0x4A:
            bit(reg_D, 1); break;
        case 0x4B:
            bit(reg_E, 1); break;
        case 0x4C:
            bit(reg_H, 1); break;
        case 0x4D:
            bit(reg_L, 1); break;
        case 0x4E:
            bit(memory_map->Read8(reg_HL.word), 1); break;
        case 0x4F:
            bit(reg_A, 1); break;
        case 0x50:
            bit(reg_B, 2); break;
        case 0x51:
            bit(reg_C, 2); break;
        case 0x52:
            bit(reg_D, 2); break;
        case 0x53:
            bit(reg_E, 2); break;
        case 0x54:
            bit(reg_H, 2); break;
        case 0x55:
            bit(reg_L, 2); break;
        case 0x56:
            bit(memory_map->Read8(reg_HL.word), 2); break;
        case 0x57:
            bit(reg_A, 2); break;
        case 0x58:
            bit(reg_B, 3); break;
        case 0x59:
            bit(reg_C, 3); break;
        case 0x5A:
            bit(reg_D, 3); break;
        case 0x5B:
            bit(reg_E, 3); break;
        case 0x5C:
            bit(reg_H, 3); break;
        case 0x5D:
            bit(reg_L, 3); break;
        case 0x5E:
            bit(memory_map->Read8(reg_HL.word), 3); break;
        case 0x5F:
            bit(reg_A, 3); break;
        case 0x60:
            bit(reg_B, 4); break;
        case 0x61:
            bit(reg_C, 4); break;
        case 0x62:
            bit(reg_D, 4); break;
        case 0x63:
            bit(reg_E, 4); break;
        case 0x64:
            bit(reg_H, 4); break;
        case 0x65:
            bit(reg_L, 4); break;
        case 0x66:
            bit(memory_map->Read8(reg_HL.word), 4); break;
        case 0x67:
            bit(reg_A, 4); break;
        case 0x68:
            bit(reg_B, 5); break;
        case 0x69:
            bit(reg_C, 5); break;
        case 0x6A:
            bit(reg_D, 5); break;
        case 0x6B:
            bit(reg_E, 5); break;
        case 0x6C:
            bit(reg_H, 5); break;
        case 0x6D:
            bit(reg_L, 5); break;
        case 0x6E:
            bit(memory_map->Read8(reg_HL.word), 5); break;
        case 0x6F:
            bit(reg_A, 5); break;
        case 0x70:
            bit(reg_B, 6); break;
        case 0x71:
            bit(reg_C, 6); break;
        case 0x72:
            bit(reg_D, 6); break;
        case 0x73:
            bit(reg_E, 6); break;
        case 0x74:
            bit(reg_H, 6); break;
        case 0x75:
            bit(reg_L, 6); break;
        case 0x76:
            bit(memory_map->Read8(reg_HL.word), 6); break;
        case 0x77:
            bit(reg_A, 6); break;
        case 0x78:
            bit(reg_B, 7); break;
        case 0x79:
            bit(reg_C, 7); break;
        case 0x7A:
            bit(reg_D, 7); break;
        case 0x7B:
            bit(reg_E, 7); break;
        case 0x7C:
            bit(reg_H, 7); break;
        case 0x7D:
            bit(reg_L, 7); break;
        case 0x7E:
            bit(memory_map->Read8(reg_HL.word), 7); break;
        case 0x7F:
            bit(reg_A, 7); break;

        // RES x, reg8
        case 0x87:
            res(reg_A, 0); break;
        case 0xBE:
            res_addr(reg_HL.word, 7); break;
        case 0xBF:
            res(reg_A, 7); break;

        // SET x, reg8
        case 0xC7:
            set(reg_A, 0); break;
        case 0xFF:
            set(reg_A, 7); break;

        default:
            logger->LogDisassembly(reg_PC.word - 2, 1);
            logger->Log(LogType::FATAL, "Unknown extended opcode!");
            gameboy->Stop();
    }

    return opcode;
}

}; // namespace Core
