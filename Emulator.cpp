#include "GameBoy.h"

#include <cstdio>


int main(int argc, char* argv[])
{
    printf("Bootrom being used: %s\n", argv[1]);
    printf("Rom being used: %s\n", argv[2]);

    ( new GameBoy(argv[1], argv[2]) )->Run();

    return 0;
}
