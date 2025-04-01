

#include "voxel_allocator.hpp"

u32* blocks[1 << (32-BLOCK_SIZE_BITS)]; // 1024

u32 freeList[FREE_LIST_SIZE];

u32 nextAllocIndex = 0;

int freeListPop = 0;
int nextFreeListIndex = 0;
int firstFreeIndex = 0;

u32 allocNode() {
    if (freeListPop--) {
        u32 node = freeList[nextFreeListIndex++];
        nextFreeListIndex ^= FREE_LIST_SIZE;
        return node;
    }
    freeListPop++;

    if (!blocks[nextAllocIndex >> BLOCK_SIZE_BITS]) {
        mallocBlock(nextAllocIndex >> BLOCK_SIZE_BITS);
    }

    nextAllocIndex += 16;
    return nextAllocIndex - 16;
}

u32 allocConsecNodes(int n) {

    if ((nextAllocIndex & BLOCK_SIZE_MASK) + n > BLOCK_SIZE) {

        freeConsecNodes(nextAllocIndex, n >> 4);
        nextAllocIndex += n;

        mallocBlock(nextAllocIndex >> BLOCK_SIZE_BITS);
    }

    nextAllocIndex += n;
    return nextAllocIndex - n;
}

void freeConsecNodes(u32 startIndex, int n) {
    if (freeListPop + n > FREE_LIST_SIZE) {
        DEBUG fprintf(stderr, "Free list full!\n");
        return;
    }
    freeListPop += n;
    while (n--) {
        freeList[firstFreeIndex++] = startIndex;
        firstFreeIndex ^= FREE_LIST_SIZE;
        startIndex += 16;
    }
}
