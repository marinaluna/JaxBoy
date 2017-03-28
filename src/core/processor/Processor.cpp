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
#include "../../common/Types.h"

#include <memory>
#include <iostream>


namespace Core {

Processor::Processor(GameBoy* gameboy, std::shared_ptr<MemoryMap>& memory_map)
:
    gameboy (gameboy),
    memory_map (memory_map)
{
    reg_A = reg_B = reg_C = reg_D = reg_E = reg_H = reg_L = 0x00;
    reg_PC = 0x0000;
    reg_SP = 0x0000;
}

int Processor::Tick()
{
    int new_cycles = ExecuteAt(reg_PC);
    return new_cycles;
}

// Decodes and executes instruction
int Processor::ExecuteAt(u16 address)
{
    u8 opcode = memory_map->Read8(address);
    u8 operand8 = memory_map->Read8(address + 1);
    u16 operand16 = memory_map->Read16(address + 1);
    bool cb_opcode = false;
    bool branch_taken = false;

    switch(opcode)
    {
        // CB
        case 0xCB:
            opcode = ExecuteCBOpcode(address + 1);
            cb_opcode = true;
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
            InterruptsEnabled = false;
            break;
        // EI
        case 0xFB:
            InterruptsEnabled = true;
            break;
        
        // LD reg8, u8
        case 0x06:
            ld_reg(reg_B, operand8);
            break;
        case 0x0E:
            ld_reg(reg_C, operand8);
            break;
        case 0x16:
            ld_reg(reg_D, operand8);
            break;
        case 0x1E:
            ld_reg(reg_E, operand8);
            break;
        case 0x26:
            ld_reg(reg_H, operand8);
            break;
        case 0x2E:
            ld_reg(reg_L, operand8);
            break;
        case 0x3E:
            ld_reg(reg_A, operand8);
            break;
        case 0x0A:
            ld_reg(reg_A, memory_map->Read8(reg_BC));
            break;
        case 0x1A:
            ld_reg(reg_A, memory_map->Read8(reg_DE));
            break;
        case 0x2A:
            ld_reg(reg_A, memory_map->Read8(reg_HL));
            reg_HL += 1;
            break;
        case 0x3A:
            ld_reg(reg_A, memory_map->Read8(reg_HL));
            reg_HL -= 1;
            break;
        case 0x46:
            ld_reg(reg_B, memory_map->Read8(operand8));
            break;
        case 0x4E:
            ld_reg(reg_C, memory_map->Read8(operand8));
            break;
        case 0x56:
            ld_reg(reg_D, memory_map->Read8(operand8));
            break;
        case 0x5E:
            ld_reg(reg_E, memory_map->Read8(operand8));
            break;
        case 0x66:
            ld_reg(reg_H, memory_map->Read8(operand8));
            break;
        case 0x6E:
            ld_reg(reg_L, memory_map->Read8(operand8));
            break;
        case 0x7E:
            ld_reg(reg_A, memory_map->Read8(operand8));
            break;
        case 0xF0:
            ld_reg(reg_A, memory_map->Read8(0xFF00+operand8));
            break;
        case 0xF2:
            ld_reg(reg_A, memory_map->Read8(0xFF00+reg_C));
            break;
        case 0x40:
            ld_reg(reg_B, reg_B);
            break;
        case 0x41:
            ld_reg(reg_B, reg_C);
            break;
        case 0x42:
            ld_reg(reg_B, reg_D);
            break;
        case 0x43:
            ld_reg(reg_B, reg_E);
            break;
        case 0x44:
            ld_reg(reg_B, reg_H);
            break;
        case 0x45:
            ld_reg(reg_B, reg_L);
            break;
        case 0x47:
            ld_reg(reg_B, reg_A);
            break;
        case 0x48:
            ld_reg(reg_C, reg_B);
            break;
        case 0x49:
            ld_reg(reg_C, reg_C);
            break;
        case 0x4A:
            ld_reg(reg_C, reg_D);
            break;
        case 0x4B:
            ld_reg(reg_C, reg_E);
            break;
        case 0x4C:
            ld_reg(reg_C, reg_H);
            break;
        case 0x4D:
            ld_reg(reg_C, reg_L);
            break;
        case 0x4F:
            ld_reg(reg_C, reg_A);
            break;
        case 0x50:
            ld_reg(reg_D, reg_B);
            break;
        case 0x51:
            ld_reg(reg_D, reg_C);
            break;
        case 0x52:
            ld_reg(reg_D, reg_D);
            break;
        case 0x53:
            ld_reg(reg_D, reg_E);
            break;
        case 0x54:
            ld_reg(reg_D, reg_H);
            break;
        case 0x55:
            ld_reg(reg_D, reg_L);
            break;
        case 0x57:
            ld_reg(reg_D, reg_A);
            break;
        case 0x58:
            ld_reg(reg_E, reg_B);
            break;
        case 0x59:
            ld_reg(reg_E, reg_C);
            break;
        case 0x5A:
            ld_reg(reg_E, reg_D);
            break;
        case 0x5B:
            ld_reg(reg_E, reg_E);
            break;
        case 0x5C:
            ld_reg(reg_E, reg_H);
            break;
        case 0x5D:
            ld_reg(reg_E, reg_L);
            break;
        case 0x5F:
            ld_reg(reg_E, reg_A);
            break;
        case 0x60:
            ld_reg(reg_H, reg_B);
            break;
        case 0x61:
            ld_reg(reg_H, reg_C);
            break;
        case 0x62:
            ld_reg(reg_H, reg_D);
            break;
        case 0x63:
            ld_reg(reg_H, reg_E);
            break;
        case 0x64:
            ld_reg(reg_H, reg_H);
            break;
        case 0x65:
            ld_reg(reg_H, reg_L);
            break;
        case 0x67:
            ld_reg(reg_H, reg_A);
            break;
        case 0x68:
            ld_reg(reg_L, reg_B);
            break;
        case 0x69:
            ld_reg(reg_L, reg_C);
            break;
        case 0x6A:
            ld_reg(reg_L, reg_D);
            break;
        case 0x6B:
            ld_reg(reg_L, reg_E);
            break;
        case 0x6C:
            ld_reg(reg_L, reg_H);
            break;
        case 0x6D:
            ld_reg(reg_L, reg_L);
            break;
        case 0x6F:
            ld_reg(reg_L, reg_A);
            break;
        case 0x78:
            ld_reg(reg_A, reg_B);
            break;
        case 0x79:
            ld_reg(reg_A, reg_C);
            break;
        case 0x7A:
            ld_reg(reg_A, reg_D);
            break;
        case 0x7B:
            ld_reg(reg_A, reg_E);
            break;
        case 0x7C:
            ld_reg(reg_A, reg_H);
            break;
        case 0x7D:
            ld_reg(reg_A, reg_L);
            break;
        case 0x7F:
            ld_reg(reg_A, reg_A);
            break;
        // LD reg16, u16
        case 0x01:
            ld_reg(reg_BC, operand16);
            break;
        case 0x11:
            ld_reg(reg_DE, operand16);
            break;
        case 0x21:
            ld_reg(reg_HL, operand16);
            break;
        case 0x31:
            ld_reg(reg_SP, operand16);
            break;
        case 0xF8:
            ld_reg(reg_HL, reg_SP + static_cast<s8>(operand8));
            break;

        // LD (addr), u8
        case 0x02:
            ld_addr(reg_BC, reg_A);
            break;
        case 0x12:
            ld_addr(reg_DE, reg_A);
            break;
        case 0x22:
            ld_addr(reg_HL, reg_A);
            reg_HL += 1;
            break;
        case 0x32:
            ld_addr(reg_HL, reg_A);
            reg_HL -= 1;
            break;
        case 0x36:
            ld_addr(reg_HL, operand8);
            break;
        case 0x70:
            ld_addr(reg_HL, reg_B);
            break;
        case 0x71:
            ld_addr(reg_HL, reg_C);
            break;
        case 0x72:
            ld_addr(reg_HL, reg_D);
            break;
        case 0x73:
            ld_addr(reg_HL, reg_E);
            break;
        case 0x74:
            ld_addr(reg_HL, reg_H);
            break;
        case 0x75:
            ld_addr(reg_HL, reg_L);
            break;
        case 0x77:
            ld_addr(reg_HL, reg_A);
            break;
        case 0xE0:
            ld_addr(0xFF00+operand8, reg_A);
            break;
        case 0xE2:
            ld_addr(0xFF00+reg_C, reg_A);
            break;
        case 0xEA:
            ld_addr(operand16, reg_A);
            break;
        // LD (addr), u16
        case 0x08:
            ld_addr(operand16, reg_SP);
            break;

        // INC reg8
        case 0x04:
            inc(reg_B);
            break;
        case 0x0C:
            inc(reg_C);
            break;
        case 0x14:
            inc(reg_D);
            break;
        case 0x1C:
            inc(reg_E);
            break;
        case 0x24:
            inc(reg_H);
            break;
        case 0x2C:
            inc(reg_L);
            break;
        case 0x3C:
            inc(reg_A);
            break;
        // INC reg16
        case 0x03:
            inc(reg_BC);
            break;
        case 0x13:
            inc(reg_DE);
            break;
        case 0x23:
            inc(reg_HL);
            break;
        case 0x33:
            inc(reg_SP);
            break;

        // DEC reg8
        case 0x05:
            dec(reg_B);
            break;
        case 0x0D:
            dec(reg_C);
            break;
        case 0x15:
            dec(reg_D);
            break;
        case 0x1D:
            dec(reg_E);
            break;
        case 0x25:
            dec(reg_H);
            break;
        case 0x2D:
            dec(reg_L);
            break;
        case 0x3D:
            dec(reg_A);
            break;
        // DEC reg16
        case 0x0B:
            dec(reg_BC);
            break;
        case 0x1B:
            dec(reg_DE);
            break;
        case 0x2B:
            dec(reg_HL);
            break;
        case 0x3B:
            dec(reg_SP);
            break;

        // ADD reg8, u8
        case 0x80:
            add(reg_A, reg_B);
            break;
        case 0x81:
            add(reg_A, reg_C);
            break;
        case 0x82:
            add(reg_A, reg_D);
            break;
        case 0x83:
            add(reg_A, reg_E);
            break;
        case 0x84:
            add(reg_A, reg_H);
            break;
        case 0x85:
            add(reg_A, reg_L);
            break;
        case 0x86:
            add(reg_A, memory_map->Read8(operand8));
            break;
        case 0x87:
            add(reg_A, reg_A);
            break;
        case 0xC6:
            add(reg_A, operand8);
            break;
        // ADD reg16, u16
        case 0x09:
            add(reg_HL, reg_BC);
            break;
        case 0x19:
            add(reg_HL, reg_DE);
            break;
        case 0x29:
            add(reg_HL, reg_HL);
            break;
        case 0x39:
            add(reg_HL, reg_SP);
            break;
        // ADD reg16, s8
        case 0xE8:
            add(reg_SP, static_cast<s8>(operand8));
            break;

        // ADC reg8, u8
        case 0x88:
            adc(reg_A, reg_B);
            break;
        case 0x89:
            adc(reg_A, reg_C);
            break;
        case 0x8A:
            adc(reg_A, reg_D);
            break;
        case 0x8B:
            adc(reg_A, reg_E);
            break;
        case 0x8C:
            adc(reg_A, reg_H);
            break;
        case 0x8D:
            adc(reg_A, reg_L);
            break;
        case 0x8E:
            adc(reg_A, memory_map->Read8(operand8));
            break;
        case 0x8F:
            adc(reg_A, reg_A);
            break;
        case 0xCE:
            adc(reg_A, operand8);
            break;

        // SUB reg8, u8
        case 0x90:
            sub(reg_A, reg_B);
            break;
        case 0x91:
            sub(reg_A, reg_C);
            break;
        case 0x92:
            sub(reg_A, reg_D);
            break;
        case 0x93:
            sub(reg_A, reg_E);
            break;
        case 0x94:
            sub(reg_A, reg_H);
            break;
        case 0x95:
            sub(reg_A, reg_L);
            break;
        case 0x96:
            sub(reg_A, memory_map->Read8(operand8));
            break;
        case 0x97:
            sub(reg_A, reg_A);
            break;
        case 0xD6:
            sub(reg_A, operand8);
            break;

        // SBC reg8, u8
        case 0x98:
            sbc(reg_A, reg_B);
            break;
        case 0x99:
            sbc(reg_A, reg_C);
            break;
        case 0x9A:
            sbc(reg_A, reg_D);
            break;
        case 0x9B:
            sbc(reg_A, reg_E);
            break;
        case 0x9C:
            sbc(reg_A, reg_H);
            break;
        case 0x9D:
            sbc(reg_A, reg_L);
            break;
        case 0x9E:
            sbc(reg_A, memory_map->Read8(operand8));
            break;
        case 0x9F:
            sbc(reg_A, reg_A);
            break;
        case 0xDE:
            sbc(reg_A, operand8);
            break;

        // AND reg8, u8
        case 0xA0:
            and8(reg_A, reg_B);
            break;
        case 0xA1:
            and8(reg_A, reg_C);
            break;
        case 0xA2:
            and8(reg_A, reg_D);
            break;
        case 0xA3:
            and8(reg_A, reg_E);
            break;
        case 0xA4:
            and8(reg_A, reg_H);
            break;
        case 0xA5:
            and8(reg_A, reg_L);
            break;
        case 0xA6:
            and8(reg_A, memory_map->Read8(operand8));
            break;
        case 0xA7:
            and8(reg_A, reg_A);
            break;
        case 0xE6:
            and8(reg_A, operand8);
            break;

        // XOR reg8, u8
        case 0xA8:
            xor8(reg_A, reg_B);
            break;
        case 0xA9:
            xor8(reg_A, reg_C);
            break;
        case 0xAA:
            xor8(reg_A, reg_D);
            break;
        case 0xAB:
            xor8(reg_A, reg_E);
            break;
        case 0xAC:
            xor8(reg_A, reg_H);
            break;
        case 0xAD:
            xor8(reg_A, reg_L);
            break;
        case 0xAE:
            xor8(reg_A, memory_map->Read8(operand8));
            break;
        case 0xAF:
            xor8(reg_A, reg_A);
            break;
        case 0xEE:
            xor8(reg_A, operand8);
            break;

        // OR reg8, u8
        case 0xB0:
            or8(reg_A, reg_B);
            break;
        case 0xB1:
            or8(reg_A, reg_C);
            break;
        case 0xB2:
            or8(reg_A, reg_D);
            break;
        case 0xB3:
            or8(reg_A, reg_E);
            break;
        case 0xB4:
            or8(reg_A, reg_H);
            break;
        case 0xB5:
            or8(reg_A, reg_L);
            break;
        case 0xB6:
            or8(reg_A, memory_map->Read8(operand8));
            break;
        case 0xB7:
            or8(reg_A, reg_A);
            break;
        case 0xF6:
            or8(reg_A, operand8);
            break;

        // RL
        case 0x17:
            rl(reg_A);
            break;

        // CP u8
        case 0xB8:
            cp(reg_B);
            break;
        case 0xB9:
            cp(reg_C);
            break;
        case 0xBA:
            cp(reg_D);
            break;
        case 0xBB:
            cp(reg_E);
            break;
        case 0xBC:
            cp(reg_H);
            break;
        case 0xBD:
            cp(reg_L);
            break;
        case 0xBE:
            cp(memory_map->Read8(reg_HL));
            break;
        case 0xBF:
            cp(reg_A);
            break;
        case 0xFE:
            cp(operand8);
            break;

        // JR s8
        case 0x18:
            jr(static_cast<s8>(operand8));
            break;
        case 0x20:
            if(!F_Zero) {
                branch_taken = true;
                jr(static_cast<s8>(operand8));
                break;
            }
            break;
        case 0x28:
            if(F_Zero) {
                branch_taken = true;
                jr(static_cast<s8>(operand8));
                break;
            }
            break;
        case 0x30:
            if(!F_Carry) {
                branch_taken = true;
                jr(static_cast<s8>(operand8));
                break;
            }
            break;
        case 0x38:
            if(F_Carry) {
                branch_taken = true;
                jr(static_cast<s8>(operand8));
                break;
            }
            break;

        // JP u16
        case 0xC3:
            jp(operand16);
            break;
        case 0xC2:
            if(!F_Zero) {
                branch_taken = true;
                jp(operand16);
                break;
            }
            break;
        case 0xCA:
            if(F_Zero) {
                branch_taken = true;
                jp(operand16);
                break;
            }
            break;
        case 0xD2:
            if(!F_Carry) {
                branch_taken = true;
                jp(operand16);
                break;
            }
            break;
        case 0xDA:
            if(F_Carry) {
                branch_taken = true;
                jp(operand16);
                break;
            }
            break;
        case 0xE9:
            jp(memory_map->Read16(reg_HL));
            break;

        // CALL u16
        case 0xCD:
            call(operand16);
            break;
        case 0xC4:
            if(!F_Zero) {
                branch_taken = true;
                call(operand16);
                break;
            }
            break;
        case 0xCC:
            if(F_Zero) {
                branch_taken = true;
                call(operand16);
                break;
            }
            break;
        case 0xD4:
            if(!F_Carry) {
                branch_taken = true;
                call(operand16);
                break;
            }
            break;
        case 0xDC:
            if(F_Carry) {
                branch_taken = true;
                call(operand16);
                break;
            }
            break;

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
            // TODO
            InterruptsEnabled = false;
            ret();
            break;

        // PUSH reg16
        case 0xC5:
            push(reg_BC);
            break;
        case 0xD5:
            push(reg_DE);
            break;
        case 0xE5:
            push(reg_HL);
            break;
        case 0xF5:
            push(reg_AF);
            break;

        // POP reg16
        case 0xC1:
            pop(reg_BC);
            break;
        case 0xD1:
            pop(reg_DE);
            break;
        case 0xE1:
            pop(reg_HL);
            break;
        case 0xF1:
            pop(reg_AF);
            break;

        default:
            std::cerr << "Unknown opcode!\n";
            gameboy->Stop();
    }

    if(cb_opcode)
    {
        // CB opcode
        if(CB_OPCODE_LOOKUP[opcode].inc_pc)
        {
            reg_PC += CB_OPCODE_LOOKUP[opcode].length;
        }
        return CB_OPCODE_LOOKUP[opcode].cycles;
    }

    if(OPCODE_LOOKUP[opcode].inc_pc)
    {
        reg_PC += OPCODE_LOOKUP[opcode].length;
    }
    return (!branch_taken)? OPCODE_LOOKUP[opcode].cycles : OPCODE_LOOKUP[opcode].cycles_branch;
}

u8 Processor::ExecuteCBOpcode(u16 address)
{
    u8 opcode = memory_map->Read8(address);
    u8 operand8 = memory_map->Read8(address + 1);
    u16 operand16 = memory_map->Read16(address + 1);

    switch(opcode)
    {
        case 0x11:
            rl(reg_C);
            break;

        case 0x7C:
            bit(reg_H, 7);
            break;

        default:
            std::cerr << "Unknown opcode!\n";
            gameboy->Stop();
    }

    return opcode;
}

}; // namespace Core
