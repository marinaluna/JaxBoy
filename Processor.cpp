#include "GameBoy.h"
#include "constants.h"
#include "memory/MemoryMap.h"

#include <cstdio>

#include "Processor.h"


#define REGHIGH(reg)			((reg & 0xFF00) >> 8)
#define REGLOW(reg)				(reg & 0xFF)
#define REGSETHIGH(reg, val)	(reg = ((val & 0xFF) << 8) | (reg & 0xFF))
#define REGSETLOW(reg, val)		(reg = (reg & 0xFF00) | val & 0xFF)

#define SETFLAG(mask)			REGSETLOW(reg_AF, REGLOW(reg_AF) | mask);
#define CLEARFLAG(mask)			REGSETLOW(reg_AF, REGLOW(reg_AF) & ~mask);

#define LDREG(reg, src)																\
		reg = src;																	\
		instruction_length = 3;														\
		cycles_this_frame = 12;
#define LDADDR(dest, src)															\
		GameBoy::GetMemory().write8(dest, src);										\
		cycles_this_frame = 8;
#define XOR(reg)																	\
		REGSETHIGH(reg_AF, REGHIGH(reg_AF) ^ reg);									\
		cycles_this_frame = 4;
#define CB(address)																	\
		ExecuteCBOpcode(address, cycles_this_frame, instruction_length);

#define BIT(mask, reg)																\
		if(reg & mask)																\
			{SETFLAG(FLAG_ZERO);}													\
		else																		\
			{CLEARFLAG(FLAG_ZERO);}													\
		instruction_length = 2;														\
		cycles_this_frame = 8;


Processor::Processor()
{
	reg_PC = 0x0000;
	reg_AF = reg_BC = reg_DE = reg_HL = 0x0000;
}

int Processor::Loop()
{
	int cycles = ExecuteNextOpcode(reg_PC);

	return cycles;
}

int Processor::ExecuteNextOpcode(uint16_t& pc_address) // Decodes and executes instruction
{
	uint8_t opcode = GameBoy::GetMemory().read8(pc_address);
	int instruction_length = 1;
	int cycles_this_frame = 1;

	switch(opcode)
	{
	case 0x00: // NOP
		break;
	case 0x21: // LD HL, imm16
		LDREG(reg_HL, GameBoy::GetMemory().read16(pc_address + 1));
		break;
	case 0x31: // LD SP, imm16
		LDREG(reg_SP, GameBoy::GetMemory().read16(pc_address + 1));
		break;
	case 0x32: // LD (HL-), A
		LDADDR(reg_HL, REGHIGH(reg_AF));
		reg_HL--;
	case 0xAF: // XOR A, A
		XOR(REGHIGH(reg_AF));
		break;
	case 0xCB: // CB extension
		CB(pc_address + 1);
		break;
	default:
		GameBoy::SystemError("Unknown OPCODE %#02x at address %#04x!\n", opcode, pc_address);
	}

	if(GameBoy::IsDebugMode && opcode != 0xCB)
	{
		if(instruction_length == 2 || instruction_length == 3)
		{
			uint16_t operand = (instruction_length == 2)? GameBoy::GetMemory().read8(pc_address + 1) : GameBoy::GetMemory().read16(pc_address + 1);
			printf("OPCODE %#02x with operand %#04x at address %#04x\n", opcode, operand, pc_address);
		}
		else
		{
			printf("OPCODE %#02x at address %#04x\n", opcode, pc_address);
		}
	}

	pc_address += instruction_length;
	return cycles_this_frame;
}

void Processor::ExecuteCBOpcode(uint16_t address, int& cycles_this_frame, int& instruction_length)
{
	int cb_opcode = GameBoy::GetMemory().read8(address);

	switch(cb_opcode)
	{
	case 0x7C: // BIT 7, H
		BIT(BIT7_MASK, REGHIGH(reg_HL));
		break;
	default:
		GameBoy::SystemError("Unknown EXTENDED OPCODE %#02x at address %#04x!\n", cb_opcode, address);
	}

	if(GameBoy::IsDebugMode)
	{
		if(instruction_length == 3 || instruction_length == 4)
		{
			uint16_t operand = (instruction_length == 3)? GameBoy::GetMemory().read8(address + 1) : GameBoy::GetMemory().read16(address + 1);
			printf("	EXTENDED OPCODE %#02x with operand %#04x at address %#04x\n", cb_opcode, operand, address);
		}
		else
		{
			printf("	EXTENDED OPCODE %#02x at address %#04x\n", cb_opcode, address);
		}
	}
}
