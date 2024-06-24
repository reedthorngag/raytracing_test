#include <stdio.h>
#include <cstdint>

int main() {

    uint8_t localpos[3] = {1, 1, 1};

    printf("%d\n",(1 << localpos[0]) << (localpos[1]<<1) << (localpos[2]<<2));
}

