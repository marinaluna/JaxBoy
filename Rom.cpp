#include <cstring> // for memcpy
#include <cstdio> // for printf

#include "Rom.h"


Rom::Rom(const char* rom_name, uint8_t* rom_bytes)
:    rom_name(rom_name)
{
    printf("Loaded ROM image: %s\n", rom_name);

    memcpy(RomInfo.Name, rom_bytes + 0x134, 0x10);
    memcpy(RomInfo.Manufacturer, rom_bytes + 0x13F, 0x4); // only used in newer carts
    RomInfo.UsesSGBFeatures = rom_bytes[0x146] == 0x03; // 3 means yes, 0 means no
    RomInfo.CartType = rom_bytes[0x147];
    /*
    Cart Type specifies which MBC type is used in the cart,
    and what external hardware (i.e. battery) is included.
    The types are as follows:
        00 - ROM only                        15 - 
        01 - MBC1                            16 - 
        02 - MBC1 + RAM                     17 - 
        03 - MBC1 + RAM + BATTERY             19 - 
        05 - MBC2                            1A - 
        06 - MBC2 + BATTERY                 1B - 
        08 - ROM+RAM                         1C - 
        09 - ROM + RAM + BATTERY             1D - 
        0B - MMM01                            1E - 
        0C - MMM01 + RAM                     20 - 
        0D - MMM01 + RAM + BATTERY             22 - 
        0F - MBC3 + TIMER + BATTERY         FC - 
        10 - MBC3 + TIMER + RAM + BATTERY     FD - 
        11 - MBC3                            FE - 
        12 - MBC3 + RAM                     FF - 
        13 - MBC3 + RAM + BATTERY
    */
    RomInfo.RomSize = rom_bytes[0x148];
    RomInfo.RamSize = rom_bytes[0x149];
    RomInfo.International = rom_bytes[0x14A] == 0x01; // 00 means Japan, 01 means international
    RomInfo.Licensee = rom_bytes[0x14B]; // if 33, SGB functions don't work
    RomInfo.VersionCode = rom_bytes[0x14C]; // usually 00

    // Save the interrupt vector code so we can replace it
    // later, after the bootrom finishes
    // TODO: This is bad
    memcpy(InterruptVectors, rom_bytes, 0x100);
}

uint8_t* Rom::GetInterruptVectorsPtr()
{
    return InterruptVectors;
}
