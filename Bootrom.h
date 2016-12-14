#pragma once

#include "Rom.h"


class Bootrom : public Rom
{
public:
	Bootrom(const char* bootrom_name);
};
