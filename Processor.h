#pragma once
#include <cstdint>


struct Reg16Bit
{
	uint8_t high;
	uint8_t low;

	inline Reg16Bit& operator=(const uint16_t rvalue)		{ high = (rvalue & 0xFF00) >> 8; low = rvalue & 0xFF; return *this; }
	inline Reg16Bit& operator=(const uint8_t rvalue)		{ high = 0x00; low = rvalue; return *this; }
	inline Reg16Bit& operator=(const Reg16Bit& rvalue)		{ high = rvalue.high; low = rvalue.low; return *this; }

	// prefix operators
	inline Reg16Bit& operator++() 							{ if(++low == 0x00) ++high; return *this; }
	inline Reg16Bit& operator--()							{ if(--low == 0xFF) --high; return *this; }
	inline uint16_t operator+(const uint16_t rvalue)		{ return uint16_t(*this) + rvalue; }
	inline uint16_t operator-(const uint16_t rvalue)		{ return uint16_t(*this) - rvalue; }
	inline Reg16Bit& operator+=(const uint16_t rvalue)		{ *this = *this + rvalue; return *this; }
	inline Reg16Bit& operator-=(const uint16_t rvalue)		{ *this = *this - rvalue; return *this; }

	inline operator uint16_t() 								{ return ((high << 8) | low); }
	inline uint8_t& High()									{ return high; }
	inline uint8_t& Low()									{ return low; }
};


class Processor
{
	// 16-bit registers; program counter and stack pointer
	Reg16Bit	reg_PC,
				reg_SP;
	// 8-bit registers; pairs are linked together to make 16-bit registers
	Reg16Bit	reg_AF;
	Reg16Bit	reg_BC;
	Reg16Bit	reg_DE;
	Reg16Bit	reg_HL;

	int Step;
	uint16_t Breakpoint;
	bool BreakpointSet;

public:
	Processor();

	int Tick();
	void DebugStep();

	int ExecuteAt(uint16_t address);
	void ExecuteCBOpcode(uint16_t address, int& cycles_this_tick, int& instruction_length);
};
