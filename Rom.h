#pragma once

#include <cstdint>


class Rom
{
protected:
	const char* rom_name;

	Rom(const char* rom_name);

public:
	virtual int getSizeInBytes() const = 0;
	virtual uint16_t getStartAddress() const = 0;
	virtual void printDebugInfo() const = 0;
};
