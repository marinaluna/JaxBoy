#include "GameBoy.h"
#include "constants.h"
#include "memory/MemoryMap.h"

#include <cstdio>

#include "Processor.h"


#define SETBIT(mask, data, value)													\
		data = ((data & ~mask) | ((value > 0)? mask : 0))
#define CHECKBIT(mask, data)														\
		(data & mask)

#define WRITE8(address, data)														\
		GameBoy::GetMemory().Write8(address, data)
#define WRITE16(address, data)														\
		GameBoy::GetMemory().Write16(address, data)
#define READ8(address)																\
		GameBoy::GetMemory().Read8(address)
#define READ16(address)																\
		GameBoy::GetMemory().Read16(address)

#define SETFLAGS(zero, sub, half, carry)											\
		if(zero != -1) SETBIT(FLAG_ZERO, reg_AF.Low(), zero);						\
		if(sub != -1) SETBIT(FLAG_SUB, reg_AF.Low(), sub);							\
		if(half != -1) SETBIT(FLAG_HALF, reg_AF.Low(), half);						\
		if(carry != -1) SETBIT(FLAG_CARRY, reg_AF.Low(), carry);
#define CHECKFLAG(mask)																\
		CHECKBIT(mask, reg_AF.Low())

#define LENGTHCYCLES(length, cycles)												\
		instruction_length = length;												\
		cycles_this_tick = cycles;


Processor::Processor()
{
	reg_PC = uint16_t(0x0000);
	reg_AF = reg_BC = reg_DE = reg_HL = uint16_t(0x0000);
}

int Processor::Tick()
{
	int cycles = ExecuteAt(reg_PC);


	// TODO
	// Fake LY
	if(READ8(0xFF40) & 0B10000000)
	{
		WRITE8(0xFF44, READ8(0xFF44) + 1);
	}
	// Fake Nintendo Logo checksum
	for(int i = 0; i < 56; i++)
	{
		WRITE8(0x104 + i, READ8(0xa8 + i));
	}

	return cycles;
}

int Processor::ExecuteAt(uint16_t address) // Decodes and executes instruction
{
	uint8_t opcode = READ8(address);
	int instruction_length;
	int cycles_this_tick;

	switch(opcode)
	{
	case 0x00: // NOP
	{
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0x01: // LD BC, imm16
	{
		reg_BC = READ16(address + 1);
		LENGTHCYCLES(3, 12);
		break;
	}
	case 0x02: // LD (BC), A
	{
		WRITE8(reg_BC, reg_AF.High());
		LENGTHCYCLES(1, 8);
		break;
	}
	case 0x03: // INC BC
	{
		++reg_BC;
		LENGTHCYCLES(1, 8);
		break;
	}
	case 0x04: // INC B
	{
		++reg_BC.High();
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0x05: // DEC B
	{
		--reg_BC.High();
		LENGTHCYCLES(1, 4);
		SETFLAGS((reg_BC.High() == 0)? 1 : 0, 1, -1, -1);
		break;
	}
	case 0x06: // LD B, imm8
	{
		reg_BC.High() = READ8(address + 1);
		LENGTHCYCLES(2, 8);
		break;
	}
	case 0x07: // RLCA
	{
		int8_t carry = CHECKBIT(BIT7_MASK, reg_AF.High());
		reg_AF.High() <<= 1;
		SETFLAGS(0, 0, 0, carry);
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0x08: // LD (imm16), SP
	{
		WRITE16(READ16(address + 1), reg_SP);
		LENGTHCYCLES(3, 20);
		break;
	}
	case 0x09: // ADD HL, BC
	{
		reg_HL += reg_BC;
		LENGTHCYCLES(1, 8);
		break;
	}
	case 0x0A: // LD A, (BC)
	{
		reg_AF = READ8(reg_BC);
		LENGTHCYCLES(1, 8);
		break;
	}
	case 0x0B: // DEC BC
	{
		--reg_BC;
		LENGTHCYCLES(1, 8);
		break;
	}
	case 0x0C: // INC C
	{
		++reg_BC.Low();
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0x0D: // DEC C
	{
		--reg_BC.Low();
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0x0E: // LD C, imm8
	{
		reg_BC.Low() = READ8(address + 1);
		LENGTHCYCLES(2, 8);
		break;
	}
	case 0x0F: // RRCA
	{
		int8_t carry = CHECKBIT(BIT0_MASK, reg_AF.High());
		reg_AF.High() >>= 1;
		SETFLAGS(0, 0, 0, carry);
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0x10: // STOP 0
	{
		// TODO: implement me
		LENGTHCYCLES(2, 4);
		break;
	}
	case 0x11: // LD DE, (imm16)
	{
		reg_DE = READ16(address + 1);
		LENGTHCYCLES(3, 12);
		break;
	}
	case 0x12: // LD (DE), A
	{
		WRITE8(reg_DE, reg_AF.High());
		LENGTHCYCLES(1, 8);
		break;
	}
	case 0x13: // INC DE
	{
		++reg_DE;
		LENGTHCYCLES(1, 8);
		break;
	}
	case 0x14: // INC D
	{
		++reg_DE.High();
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0x15: // DEC D
	{
		--reg_DE.High();
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0x16: // LD D, imm8
	{
		reg_DE.High() = READ8(address + 1);
		LENGTHCYCLES(2, 8);
		break;
	}
	case 0x17: // RLA
	{
		int8_t carry = CHECKBIT(BIT7_MASK, reg_AF.High());
		reg_AF.High() <<= 1;
		SETBIT(BIT0_MASK, reg_AF.High(), CHECKFLAG(FLAG_CARRY));
		SETFLAGS(0, 0, 0, carry);
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0x18: // JR imm8
	{
		reg_PC.operator+=((int16_t)READ8(address + 1));
		LENGTHCYCLES(2, 12);
		break;
	}
	case 0x19: // ADD HL, DE
	{
		reg_HL += reg_DE;
		LENGTHCYCLES(1, 8);
		break;
	}
	case 0x1A: // LD A, (DE)
	{
		reg_AF.High() = READ8(reg_DE);
		LENGTHCYCLES(1, 8);
		break;
	}
	case 0x1B: // DEC DE
	{
		--reg_DE;
		LENGTHCYCLES(1, 8);
		break;
	}
	case 0x1C: // INC E
	{
		++reg_DE.Low();
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0x1D: // DEC E
	{
		--reg_DE.Low();
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0x1E: // LD E, imm8
	{
		reg_DE.Low() = READ8(address + 1);
		LENGTHCYCLES(2, 8);
		break;
	}
	case 0x1F: // RRA
	{
		int8_t carry = CHECKBIT(BIT0_MASK, reg_AF.High());
		reg_AF.High() >>= 1;
		SETBIT(BIT7_MASK, reg_AF.High(), CHECKFLAG(FLAG_CARRY));
		SETFLAGS(0, 0, 0, carry);
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0x20: // JR NZ, imm8
	{
		if(CHECKFLAG(FLAG_ZERO) == 0)
		{
			reg_PC = (uint16_t)((uint16_t)reg_PC + (int8_t)READ8(address + 1));
			LENGTHCYCLES(2, 12);
		}
		else
		{
			LENGTHCYCLES(2, 8);
		}

		break;
	}
	case 0x21: // LD HL, imm16
	{
		reg_HL = READ16(address + 1);
		LENGTHCYCLES(3, 12);
		break;
	}
	case 0x22: // LD (HL+), A
	{
		WRITE8(reg_HL, reg_AF.High());
		++reg_HL;
		LENGTHCYCLES(1, 8);
		break;
	}
	case 0x23: // INC HL
	{
		++reg_HL;
		LENGTHCYCLES(1, 2);
		break;
	}
	case 0x24: // INC H
	{
		++reg_HL.High();
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0x25: // DEC H
	{
		--reg_HL.High();
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0x26: // LD H, imm8
	{
		reg_HL.High() = READ8(address + 1);
		LENGTHCYCLES(2, 8);
		break;
	}
	case 0x27: // DAA
	{
		// TODO
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0x28: // JR Z, imm8
	{
		if(CHECKFLAG(FLAG_ZERO) != 0)
		{
			reg_PC = (uint16_t)((uint16_t)reg_PC + (int8_t)READ8(address + 1));
			LENGTHCYCLES(2, 12);
		}
		else
		{
			LENGTHCYCLES(2, 8);
		}
		break;
	}
	case 0x29: // ADD HL, HL
	{
		reg_HL += reg_HL;
		LENGTHCYCLES(1, 8);
		break;
	}
	case 0x2A: // LD A, (HL+)
	{
		reg_AF.High() = READ8(reg_HL);
		++reg_HL;
		LENGTHCYCLES(1, 8);
		break;
	}
	case 0x2B: // DEC HL
	{
		--reg_HL;
		LENGTHCYCLES(1, 8);
		break;
	}
	case 0x2C: // INC L
	{
		++reg_HL.Low();
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0x2D: // DEC L
	{
		--reg_HL.Low();
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0x2E: // LD L, imm8
	{
		reg_HL.Low() = READ8(address + 1);
		LENGTHCYCLES(2, 8);
		break;
	}
	case 0x2F: // CPL
	{
		// TODO
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0x30: // JR NC, imm8
	{
		if(CHECKFLAG(FLAG_CARRY) == 0)
		{
			reg_PC = (uint16_t)((uint16_t)reg_PC + (int8_t)READ8(address + 1));
			LENGTHCYCLES(2, 12);
		}
		else
		{
			LENGTHCYCLES(2, 8);
		}
		break;
	}
	case 0x31: // LD SP, imm16
	{
		reg_SP = READ16(address + 1);
		LENGTHCYCLES(3, 12);
		break;
	}
	case 0x32: // LD (HL-), A
	{
		WRITE8(reg_HL, reg_AF.High());
		--reg_HL;
		LENGTHCYCLES(1, 8);
		break;
	}
	case 0x33: // INC SP
	{
		++reg_SP;
		LENGTHCYCLES(1, 8);
		break;
	}
	case 0x34: // INC (HL)
	{
		WRITE8(reg_HL, READ8(reg_HL) + 1);
		LENGTHCYCLES(1, 12);
		break;
	}
	case 0x35: // DEC (HL)
	{
		WRITE8(reg_HL, READ8(reg_HL) - 1);
		LENGTHCYCLES(1, 12);
		break;
	}
	case 0x36: // LD (HL), imm8
	{
		WRITE8(reg_HL, READ8(address + 1));
		LENGTHCYCLES(2, 12);
		break;
	}
	case 0x37: // SCF
	{
		// TODO
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0x38: // JR C, imm8
	{
		if(CHECKFLAG(FLAG_CARRY) != 0)
		{
			reg_PC = (uint16_t)((uint16_t)reg_PC + (int8_t)READ8(address + 1));
			LENGTHCYCLES(2, 12);
		}
		else
		{
			LENGTHCYCLES(2, 8);
		}
		break;
	}
	case 0x39: // ADD HL, SP
	{
		reg_HL += reg_SP;
		LENGTHCYCLES(1, 8);
		break;
	}
	case 0x3A: // LD A, (HL-)
	{
		reg_AF.High() = READ8(reg_HL);
		--reg_HL;
		LENGTHCYCLES(1, 8);
		break;
	}
	case 0x3B: // DEC SP
	{
		--reg_SP;
		LENGTHCYCLES(1, 8);
		break;
	}
	case 0x3C: // INC A
	{
		++reg_AF.High();
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0x3D: // DEC A
	{
		--reg_AF.High();
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0x3E: // LD A, imm8
	{
		reg_AF.High() = READ8(address + 1);
		LENGTHCYCLES(2, 8);
		break;
	}
	case 0x3F: // CCF
	{
		// TODO
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0x40: // LD B, B
	{
		reg_BC.High() = reg_BC.High();
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0x41: // LD B, C
	{
		reg_BC.High() = reg_BC.Low();
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0x42: // LD B, D
	{
		reg_BC.High() = reg_DE.High();
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0x43: // LD B, E
	{
		reg_BC.High() = reg_DE.Low();
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0x44: // LD B, H
	{
		reg_BC.High() = reg_HL.High();
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0x45: // LD B, L
	{
		reg_BC.High() = reg_HL.Low();
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0x46: // LD B, (HL)
	{
		reg_BC.High() = READ8(reg_HL);
		LENGTHCYCLES(1, 8);
		break;
	}
	case 0x47: // LD B, A
	{
		reg_BC.High() = reg_AF.High();
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0x48: // LD C, B
	{
		reg_BC.Low() = reg_BC.High();
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0x49: // LD C, C
	{
		reg_BC.Low() = reg_BC.Low();
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0x4A: // LD C, D
	{
		reg_BC.Low() = reg_DE.High();
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0x4B: // LD C, E
	{
		reg_BC.Low() = reg_DE.Low();
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0x4C: // LD C, H
	{
		reg_BC.Low() = reg_HL.High();
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0x4D: // LD C, L
	{
		reg_BC.Low() = reg_HL.Low();
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0x4E: // LD C, (HL)
	{
		reg_BC.Low() = READ8(reg_HL);
		LENGTHCYCLES(1, 8);
		break;
	}
	case 0x4F: // LD C, A
	{
		reg_BC.Low() = reg_AF.High();
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0x50: // LD D, B
	{
		reg_DE.High() = reg_BC.High();
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0x51: // LD D, C
	{
		reg_DE.High() = reg_BC.Low();
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0x52: // LD D, D
	{
		reg_DE.High() = reg_DE.High();
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0x53: // LD D, E
	{
		reg_DE.High() = reg_DE.Low();
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0x54: // LD D, H
	{
		reg_DE.High() = reg_HL.High();
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0x55: // LD D, L
	{
		reg_DE.High() = reg_HL.Low();
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0x56: // LD D, (HL)
	{
		reg_DE.High() = READ8(reg_HL);
		LENGTHCYCLES(1, 8);
		break;
	}
	case 0x57: // LD D, A
	{
		reg_DE.High() = reg_AF.High();
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0x58: // LD E, B
	{
		reg_DE.Low() = reg_BC.High();
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0x59: // LD E, C
	{
		reg_DE.Low() = reg_BC.Low();
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0x5A: // LD E, D
	{
		reg_DE.Low() = reg_DE.High();
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0x5B: // LD E, E
	{
		reg_DE.Low() = reg_DE.Low();
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0x5C: // LD E, H
	{
		reg_DE.Low() = reg_HL.High();
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0x5D: // LD E, L
	{
		reg_DE.Low() = reg_HL.Low();
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0x5E: // LD E, (HL)
	{
		reg_DE.Low() = READ8(reg_HL);
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0x5F: // LD E, A
	{
		reg_DE.Low() = reg_AF.High();
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0x60: // LD H, B
	{
		reg_HL.High() = reg_BC.High();
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0x61: // LD H, C
	{
		reg_HL.High() = reg_BC.Low();
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0x62: // LD H, D
	{
		reg_HL.High() = reg_DE.High();
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0x63: // LD H, E
	{
		reg_HL.High() = reg_DE.Low();
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0x64: // LD H, H
	{
		reg_HL.High() = reg_HL.High();
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0x65: // LD H, L
	{
		reg_HL.High() = reg_HL.Low();
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0x66: // LD H, (HL)
	{
		reg_HL.High() = READ8(reg_HL);
		LENGTHCYCLES(1, 8);
		break;
	}
	case 0x67: // LD H, A
	{
		reg_HL.High() = reg_AF.High();
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0x68: // LD L, B
	{
		reg_HL.Low() = reg_BC.High();
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0x69: // LD L, C
	{
		reg_HL.Low() = reg_BC.Low();
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0x6A: // LD L, D
	{
		reg_HL.Low() = reg_DE.High();
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0x6B: // LD L, E
	{
		reg_HL.Low() = reg_DE.Low();
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0x6C: // LD L, H
	{
		reg_HL.Low() = reg_HL.High();
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0x6D: // LD L, L
	{
		reg_HL.Low() = reg_HL.Low();
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0x6E: // LD L, (HL)
	{
		reg_HL.Low() = READ8(reg_HL);
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0x6F: // LD L, A
	{
		reg_HL.Low() = reg_AF.High();
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0x70: // LD (HL), B
	{
		WRITE8(reg_HL, reg_BC.High());
		LENGTHCYCLES(1, 8);
		break;
	}
	case 0x71: // LD (HL), C
	{
		WRITE8(reg_HL, reg_BC.Low());
		LENGTHCYCLES(1, 8);
		break;
	}
	case 0x72: // LD (HL), D
	{
		WRITE8(reg_HL, reg_DE.High());
		LENGTHCYCLES(1, 8);
		break;
	}
	case 0x73: // LD (HL), E
	{
		WRITE8(reg_HL, reg_DE.Low());
		LENGTHCYCLES(1, 8);
		break;
	}
	case 0x74: // LD (HL), H
	{
		WRITE8(reg_HL, reg_HL.High());
		LENGTHCYCLES(1, 8);
		break;
	}
	case 0x75: // LD (HL), L
	{
		WRITE8(reg_HL, reg_HL.Low());
		LENGTHCYCLES(1, 8);
		break;
	}
	case 0x76: // HALT
	{
		// TODO
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0x77: // LD (HL), A
	{
		WRITE8(reg_HL, reg_AF.High());
		LENGTHCYCLES(1, 8);
		break;
	}
	case 0x78: // LD A, B
	{
		reg_AF.High() = reg_BC.High();
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0x79: // LD A, C
	{
		reg_AF.High() = reg_BC.Low();
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0x7A: // LD A, D
	{
		reg_AF.High() = reg_DE.High();
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0x7B: // LD A, E
	{
		reg_AF.High() = reg_DE.Low();
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0x7C: // LD A, H
	{
		reg_AF.High() = reg_HL.High();
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0x7D: // LD A, L
	{
		reg_AF.High() = reg_HL.Low();
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0x7E: // LD A, (HL)
	{
		reg_AF.High() = READ8(reg_HL);
		LENGTHCYCLES(1, 8);
		break;
	}
	case 0x7F: // LD A, A
	{
		reg_AF.High() = reg_AF.High();
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0x80: // ADD A, B
	{
		reg_AF.High() += reg_BC.High();
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0x81: // ADD A, C
	{
		reg_AF.High() += reg_BC.Low();
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0x82: // ADD A, D
	{
		reg_AF.High() += reg_DE.High();
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0x83: // ADD A, E
	{
		reg_AF.High() += reg_DE.Low();
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0x84: // ADD A, H
	{
		reg_AF.High() += reg_HL.High();
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0x85: // ADD A, L
	{
		reg_AF.High() += reg_HL.Low();
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0x86: // ADD A, (HL)
	{
		reg_AF.High() += READ8(reg_HL);
		LENGTHCYCLES(1, 8);
		break;
	}
	case 0x87: // ADD A, A
	{
		reg_AF.High() += reg_AF.High();
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0x88: // ADC A, B
	{
		reg_AF.High() += reg_BC.High() + (CHECKFLAG(FLAG_CARRY)? 1 : 0);
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0x89: // ADC A, C
	{
		reg_AF.High() += reg_BC.Low() + (CHECKFLAG(FLAG_CARRY)? 1 : 0);
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0x8A: // ADC A, D
	{
		reg_AF.High() += reg_DE.High() + (CHECKFLAG(FLAG_CARRY)? 1 : 0);
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0x8B: // ADC A, E
	{
		reg_AF.High() += reg_DE.Low() + (CHECKFLAG(FLAG_CARRY)? 1 : 0);
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0x8C: // ADC A, H
	{
		reg_AF.High() += reg_HL.High() + (CHECKFLAG(FLAG_CARRY)? 1 : 0);
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0x8D: // ADC A, L
	{
		reg_AF.High() += reg_HL.Low() + (CHECKFLAG(FLAG_CARRY)? 1 : 0);
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0x8E: // ADC A, (HL)
	{
		reg_AF.High() += READ8(reg_HL) + (CHECKFLAG(FLAG_CARRY)? 1 : 0);
		LENGTHCYCLES(1, 8);
		break;
	}
	case 0x8F: // ADC A, A
	{
		reg_AF.High() += reg_AF.High() + (CHECKFLAG(FLAG_CARRY)? 1 : 0);
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0x90: // SUB B
	{
		reg_AF.High() -= reg_BC.High();
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0x91: // SUB C
	{
		reg_AF.High() -= reg_BC.Low();
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0x92: // SUB D
	{
		reg_AF.High() -= reg_DE.High();
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0x93: // SUB E
	{
		reg_AF.High() -= reg_DE.Low();
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0x94: // SUB H
	{
		reg_AF.High() -= reg_HL.High();
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0x95: // SUB L
	{
		reg_AF.High() -= reg_HL.Low();
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0x96: // SUB (HL)
	{
		reg_AF.High() -= READ8(reg_HL);
		LENGTHCYCLES(1, 8);
		break;
	}
	case 0x97: // SUB A
	{
		reg_AF.High() -= reg_AF.High();
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0x98: // SBC A, B
	{
		reg_AF.High() -= reg_BC.High() + (CHECKFLAG(FLAG_CARRY)? 1 : 0);
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0x99: // SBC A, C
	{
		reg_AF.High() -= reg_BC.Low() + (CHECKFLAG(FLAG_CARRY)? 1 : 0);
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0x9A: // SBC A, D
	{
		reg_AF.High() -= reg_DE.High() + (CHECKFLAG(FLAG_CARRY)? 1 : 0);
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0x9B: // SBC A, E
	{
		reg_AF.High() -= reg_DE.Low() + (CHECKFLAG(FLAG_CARRY)? 1 : 0);
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0x9C: // SBC A, H
	{
		reg_AF.High() -= reg_HL.High() + (CHECKFLAG(FLAG_CARRY)? 1 : 0);
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0x9D: // SBC A, L
	{
		reg_AF.High() -= reg_HL.Low() + (CHECKFLAG(FLAG_CARRY)? 1 : 0);
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0x9E: // SBC A, (HL)
	{
		reg_AF.High() -= READ8(reg_HL) + (CHECKFLAG(FLAG_CARRY)? 1 : 0);
		LENGTHCYCLES(1, 8);
		break;
	}
	case 0x9F: // SBC A, A
	{
		reg_AF.High() -= reg_AF.High() + (CHECKFLAG(FLAG_CARRY)? 1 : 0);
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0xA0: // AND B
	{
		reg_AF.High() &= reg_BC.High();
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0xA1: // AND C
	{
		reg_AF.High() &= reg_BC.Low();
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0xA2: // AND D
	{
		reg_AF.High() &= reg_DE.High();
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0xA3: // AND E
	{
		reg_AF.High() &= reg_DE.Low();
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0xA4: // AND H
	{
		reg_AF.High() &= reg_HL.High();
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0xA5: // AND L
	{
		reg_AF.High() &= reg_HL.Low();
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0xA6: // AND (HL)
	{
		reg_AF.High() &= READ8(reg_HL);
		LENGTHCYCLES(1, 8);
		break;
	}
	case 0xA7: // AND A
	{
		reg_AF.High() &= reg_AF.High();
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0xA8: // XOR B
	{
		reg_AF.High() ^= reg_BC.High();
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0xA9: // XOR C
	{
		reg_AF.High() ^= reg_BC.Low();
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0xAA: // XOR D
	{
		reg_AF.High() ^= reg_DE.High();
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0xAB: // XOR E
	{
		reg_AF.High() ^= reg_DE.Low();
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0xAC: // XOR H
	{
		reg_AF.High() ^= reg_HL.High();
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0xAD: // XOR L
	{
		reg_AF.High() ^= reg_HL.Low();
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0xAE: // XOR (HL)
	{
		reg_AF.High() ^= READ8(reg_HL);
		LENGTHCYCLES(1, 8);
		break;
	}
	case 0xAF: // XOR A, A
	{
		reg_AF.High() ^= reg_AF.High();
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0xB0: // OR B
	{
		reg_AF.High() |= reg_BC.High();
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0xB1: // OR C
	{
		reg_AF.High() |= reg_BC.Low();
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0xB2: // OR D
	{
		reg_AF.High() |= reg_DE.High();
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0xB3: // OR E
	{
		reg_AF.High() |= reg_DE.Low();
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0xB4: // OR H
	{
		reg_AF.High() |= reg_HL.High();
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0xB5: // OR L
	{
		reg_AF.High() |= reg_HL.Low();
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0xB6: // OR (HL)
	{
		reg_AF.High() |= READ8(reg_HL);
		LENGTHCYCLES(1, 8);
		break;
	}
	case 0xB7: // OR A
	{
		reg_AF.High() |= reg_AF.High();
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0xB8: // CP B
	{
		SETFLAGS((reg_AF.High() == reg_BC.High())? 1: 0, 1, -1, -1);
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0xB9: // CP C
	{
		SETFLAGS((reg_AF.High() == reg_BC.Low())? 1: 0, 1, -1, -1);
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0xBA: // CP D
	{
		SETFLAGS((reg_AF.High() == reg_DE.High())? 1: 0, 1, -1, -1);
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0xBB: // CP E
	{
		SETFLAGS((reg_AF.High() == reg_DE.Low())? 1: 0, 1, -1, -1);
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0xBC: // CP H
	{
		SETFLAGS((reg_AF.High() == reg_HL.High())? 1: 0, 1, -1, -1);
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0xBD: // CP L
	{
		SETFLAGS((reg_AF.High() == reg_HL.Low())? 1: 0, 1, -1, -1);
		LENGTHCYCLES(1, 4);
		break;
	}
	case 0xBE: // CP (HL)
	{
		SETFLAGS((reg_AF.High() == READ8(reg_HL))? 1: 0, 1, -1, -1);
		LENGTHCYCLES(1, 8);
		break;
	}
	case 0xBF: // CP A
	{
		SETFLAGS((reg_AF.High() == reg_AF.High())? 1: 0, 1, -1, -1);
		LENGTHCYCLES(1, 4);
		break;
	}

	case 0xC1: // POP BC
	{
		reg_BC = READ16(reg_SP);
		reg_SP += 2;
		LENGTHCYCLES(1, 12);
		break;
	}
	case 0xC5: // PUSH BC
	{
		reg_SP -= 2;
		WRITE16(reg_SP, reg_BC);
		LENGTHCYCLES(1, 16);
		break;
	}
	case 0xC9: // RET
	{
		reg_PC = READ16(reg_SP);
		reg_SP += 2;
		LENGTHCYCLES(1, 16);
		break;
	}
	case 0xCB: // CB extension
	{
		ExecuteCBOpcode(address + 1, cycles_this_tick, instruction_length);
		break;
	}
	case 0xCD: // CALL imm16
	{
		reg_SP -= 2;
		WRITE16(reg_SP, reg_PC);
		reg_PC = READ16(address + 1);

		LENGTHCYCLES(0, 24);
		break;
	}

	case 0xE0: // LDH (imm8), A
	{
		WRITE8(0xFF00 + READ8(address + 1), reg_AF.High());
		LENGTHCYCLES(2, 12);
		break;
	}
	case 0xE2: // LD A, (C)
	{
		reg_AF.High() = READ8(0xFF00 + reg_BC.Low());
		LENGTHCYCLES(2, 8);
		break;
	}
	case 0xEA: // LD (imm16), A
	{
		WRITE8(READ16(address + 1), reg_AF.High());
		LENGTHCYCLES(3, 16);
		break;
	}

	case 0xF0: // LDH A, (imm8)
	{
		reg_AF.High() = READ8(0xFF00 + READ8(address + 1));
		LENGTHCYCLES(2, 12);
		break;
	}
	case 0xFE: // CP imm8
	{
		SETFLAGS((reg_AF.High() == READ8(address + 1))? 1: 0, 1, -1, -1);
		LENGTHCYCLES(2, 8);
		break;
	}

	default:
		GameBoy::SystemError("Unknown OPCODE %#02x at address %#04x!\n", opcode, address);
	}

	if(GameBoy::IsDebugMode && opcode != 0xCB)
	{
		if(instruction_length == 2 || instruction_length == 3)
		{
			uint16_t operand = (instruction_length == 2)? READ8(address + 1) : READ16(address + 1);
			printf("OPCODE %#02x with operand %#04x at address %#04x\n", opcode, operand, address);
		}
		else
		{
			printf("OPCODE %#02x at address %#04x\n", opcode, address);
		}
	}

	reg_PC += (uint16_t)instruction_length;
	return cycles_this_tick;
}

void Processor::ExecuteCBOpcode(uint16_t address, int& cycles_this_tick, int& instruction_length)
{
	int cb_opcode = READ8(address);

	switch(cb_opcode)
	{
	case 0x11: // RL C
	{
		int8_t carry = CHECKBIT(BIT7_MASK, reg_BC.Low());
		reg_BC.Low() <<= 1;
		SETBIT(BIT0_MASK, reg_BC.Low(), CHECKFLAG(FLAG_CARRY));
		SETFLAGS(0, 0, 0, carry);
		LENGTHCYCLES(2, 8);
		break;
	}
	case 0x7C: // BIT 7, H
	{
		SETFLAGS(CHECKBIT(BIT7_MASK, reg_HL.High()), 0, 1, -1);
		LENGTHCYCLES(2, 8);
		break;
	}
	default:
		GameBoy::SystemError("Unknown EXTENDED OPCODE %#02x at address %#04x!\n", cb_opcode, address);
	}

	if(GameBoy::IsDebugMode)
	{
		if(instruction_length == 3 || instruction_length == 4)
		{
			uint16_t operand = (instruction_length == 3)? READ8(address + 1) : READ16(address + 1);
			printf("	EXTENDED OPCODE %#02x with operand %#04x at address %#04x\n", cb_opcode, operand, address);
		}
		else
		{
			printf("	EXTENDED OPCODE %#02x at address %#04x\n", cb_opcode, address);
		}
	}
}
