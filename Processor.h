#pragma once

#include <cstdint>


class Processor
{
	// 16-bit registers; program counter and stack pointer
	uint16_t	reg_PC,
				reg_SP;
	// 8-bit registers; pairs are linked together to make 16-bit registers
	uint16_t	reg_AF;
	uint16_t	reg_BC;
	uint16_t	reg_DE;
	uint16_t	reg_HL;

public:
	Processor();

	int Loop();
	int ExecuteNextOpcode(uint16_t& address);
	void ExecuteCBOpcode(uint16_t address, int& cycles_this_frame, int& instruction_length);
};
