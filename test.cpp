#include <stdio.h>
#include <stdint.h>

struct Pos {
    int x;
    int y;
    int z;
};

int getMortonPos(Pos pos) {

    int n = pos.x;
    n = (n | (n << 16)) & 0x030000FF;
    n = (n | (n <<  8)) & 0x0300F00F;
    //n = (n | (n <<  4)) & 0x030C30C3;
    int x = n;

    n = pos.y;
    n = (n | (n << 16)) & 0x030000FF;
    n = (n | (n <<  8)) & 0x0300F00F;
    //n = (n | (n <<  4)) & 0x030C30C3;
    int y = n;

    n = pos.z;
    n = (n | (n << 16)) & 0x030000FF;
    n = (n | (n <<  8)) & 0x0300F00F;
    //n = (n | (n <<  4)) & 0x030C30C3;

    return (n << 8) | (y << 4) | x;
}

int main() {

    Pos pos{0x35,0x35,0x35};

    uint32_t m = getMortonPos(pos);
    printf("%x\n",m);
    return 0;

    uint64_t n = 0x33;

    // 0nc = 0b1100

    n = (n | (n << 16)) & 0x030000FF;
    n = (n | (n <<  8)) & 0x0300F00F;
    n = (n | (n <<  4)) & 0x030C30C3;

    
    printf("00%llx\n",n);

    n = 0x33;

    n = (n | (n << 18)) &  0x0c0000ff;
    n = (n | (n <<  10)) & 0x0c00f00f;
    n = (n | (n <<  6)) &  0x0c30c30c;
    n = (n | (n <<  2)) &  0x0c30c30c;

    printf("%llx\n",n);

    n = 0x33;

    n = (n | (n << 20)) &  0x300000ff;
    n = (n | (n <<  12)) & 0x3003c00f;
    n = (n | (n <<  8)) &  0x30c30c03;
    n = (n | (n <<  4)) &  0x30c30c30;
    printf("0%llx\n",n);
    n |= pos.x | pos.y;

    printf("%llx\n",n);
    return 0;
}