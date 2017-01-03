#include "GameBoy.h"
#include "constants.h"
#include "memory/MemoryMap.h"

#include <cstdio>

#include "Processor.h"


#define SETBIT(mask, data, value)													\
		data = ((data & ~mask) | ((value)? mask : 0))
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
		if(zero != -1) SETBIT(BIT7_MASK, reg_AF.High(), zero)						\
		if(sub != -1) SETBIT(BIT6_MASK, reg_AF.High(), sub)							\
		if(half != -1) SETBIT(BIT5_MASK, reg_AF.High(), half)						\
		if(carry != -1) SETBIT(BIT4_MASK, reg_AF.High(), carry)
#define CHECKFLAG(mask)																\
		CHECKBIT(mask, reg_AF.High())

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

	return cycles;
}

int Processor::ExecuteAt(uint16_t address) // Decodes and executes instruction
{
	uint8_t opcode = GameBoy::GetMemory().Read8(address);
	int instruction_length;
	int cycles_this_tick;

	switch(opcode)
	{
	case 0x00: // NOP
		break;
	case 0x01: // LD BC, imm16
		reg_BC = READ16(address + 1);
		LENGTHCYCLES(3, 12);
		break;
	case 0x02: // LD (BC), A
		WRITE8(reg_BC, reg_AF.High());
		LENGTHCYCLES(1, 8);
		break;
	case 0x03: // INC BC
		++reg_BC;
		LENGTHCYCLES(1, 8);
		break;
	case 0x04: // INC B
		++reg_BC.High();
		LENGTHCYCLES(1, 4);
		break;
	case 0x05: // DEC B
		--reg_BC.High();
		LENGTHCYCLES(1, 4);
		break;
	case 0x06: // LD B, imm8
		reg_BC.High() = READ8(address + 1);
		LENGTHCYCLES(2, 8);
		break;
	case 0x07: // RLCA
		break;
	case 0x08: // LD (imm16), SP
	case 0x09: // ADD HL, BC
	case 0x0A: // LD A, (BC)
	case 0x0B: // DEC BC
	case 0x0C: // INC C
	case 0x0D: // DEC C
	case 0x0E: // LD C, imm8
	case 0x0F: // RRCA
	case 0x21: // LD HL, imm16
		reg_HL = READ16(address + 1);
		LENGTHCYCLES(3, 12);
		break;
	case 0x31: // LD SP, imm16
		reg_SP = READ16(address + 1);
		LENGTHCYCLES(3, 12);
		break;
	case 0x32: // LD (HL-), A
		WRITE8(reg_HL, reg_AF.High());
		--reg_HL;
		LENGTHCYCLES(1, 8);
		break;
	case 0xAF: // XOR A, A
		reg_AF.High() ^= reg_AF.High();
		LENGTHCYCLES(1, 4);
		break;
	case 0xCB: // CB extension
		ExecuteCBOpcode(address + 1, cycles_this_tick, instruction_length);
		break;
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

	reg_PC += instruction_length;
	return cycles_this_tick;
}

void Processor::ExecuteCBOpcode(uint16_t address, int& cycles_this_tick, int& instruction_length)
{
	int cb_opcode = READ8(address);

	switch(cb_opcode)
	{
	case 0x7C: // BIT 7, H
		SETFLAGS(CHECKBIT(BIT7_MASK, reg_HL.High()), 0, 1, -1);
		LENGTHCYCLES(2, 8);
		break;
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
