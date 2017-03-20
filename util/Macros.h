#pragma once

#define MEMORY_MAP_SIZE					0x10000
#define PGROM_OFFSET					0x0000
#define VRAM_OFFSET					0x8000
#define EWRAM_OFFSET					0xA000
#define WRAM_OFFSET					0xC000
#define FORBIDDEN1_AREA_OFFSET				0xE000
#define OAM_OFFSET					0xFE00
#define FORBIDDEN2_AREA_OFFSET				0xFEA0
#define MMIO_OFFSET					0xFF00
#define HIGH_RAM_OFFSET					0xFF80

#define BIT0_MASK					0b00000001
#define BIT1_MASK					0b00000010
#define BIT2_MASK					0b00000100
#define BIT3_MASK					0b00001000
#define BIT4_MASK					0b00010000
#define BIT5_MASK					0b00100000
#define BIT6_MASK					0b01000000
#define BIT7_MASK					0b10000000

#define SETBIT(mask, data, value)													\
		data = ((data & ~mask) | ((value > 0)? mask : 0))
#define CHECKBIT(mask, data)														\
		(data & mask)
