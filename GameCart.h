#pragma once

#include "Rom.h"


class GameCart : public Rom
{
public:
	GameCart(const char* rom_name);

	virtual int getSizeInBytes() const;
	virtual uint16_t getStartAddress() const;
	virtual void printDebugInfo() const;
};
