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
    n = (n | (n <<  4)) & 0x030C30C3;
    int x = n;

    n = pos.y;
    n = (n | (n << 16)) & 0x030000FF;
    n = (n | (n <<  8)) & 0x0300F00F;
    n = (n | (n <<  4)) & 0x030C30C3;
    int y = n;

    n = pos.z;
    n = (n | (n << 16)) & 0x030000FF;
    n = (n | (n <<  8)) & 0x0300F00F;
    n = (n | (n <<  4)) & 0x030C30C3;

    return (n << 4) | (y << 2) | x;
}

int main() {

    Pos pos{0x34,0x33,0x33};

    int m = getMortonPos(pos);
    printf("%x\n",m);

    int i = 1;
    int posOffset = i * 2;
    int posOffset2 = i * 6;

    for (int i = 0; i < 10; i++) {
        pos.z++;
        int n = pos.z;
        n = (n | (n << 16)) & 0x030000FF;
        n = (n | (n <<  8)) & 0x0300F00F;
        n = (n | (n <<  4)) & 0x030C30C3;
        m &= 0xFCF3CF3C << 4 | 0xf;
        m |= n << 4;

        Pos curr = Pos{(pos.x >> posOffset) & 0x3,(pos.y >> posOffset) & 0x3,(pos.z >> posOffset) & 0x3};
        int index = curr.z << 4 | curr.y << 2 | curr.x;

        int index2 = (m >> posOffset2) & 0x3f;

        if (m != getMortonPos(pos)) {
            printf("%d %x %x\n",i,m,getMortonPos(pos));
        }
    }

    return 0;
}