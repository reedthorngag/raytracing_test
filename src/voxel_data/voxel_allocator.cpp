
#include "voxel_allocator.hpp"


u8* blocks[1 << (32-BLOCK_SIZE_BITS)]; // 1024

u32 freeList[FREE_LIST_SIZE_MASK];

u32 nextAllocIndex = 0;

int freeListPop = 0;
int nextFreeListIndex = 0;
int firstFreeIndex = 0;

Ptr allocNode() {
    if (freeListPop--) {
        u32 node = freeList[nextFreeListIndex++];
        nextFreeListIndex &= FREE_LIST_SIZE_MASK;
        DEBUG(5) printf("node found in free list: %u free list index: %d\n",node,nextFreeListIndex);
        return convertToPtr(node);
    }
    freeListPop++;

    if (!blocks[nextAllocIndex >> BLOCK_SIZE_BITS]) {
        mallocBlock(nextAllocIndex >> BLOCK_SIZE_BITS);
    }

    Ptr ptr = convertToPtr(nextAllocIndex);
    nextAllocIndex += 16;
    return ptr;
}

Ptr allocConsecNodes(int n) {

    if ((nextAllocIndex & BLOCK_SIZE_MASK) + n > BLOCK_SIZE) {

        freeConsecNodes(nextAllocIndex, n >> 4);
        nextAllocIndex += n;

        mallocBlock(nextAllocIndex >> BLOCK_SIZE_BITS);
    }

    Ptr ptr = convertToPtr(nextAllocIndex);
    nextAllocIndex += n;
    return ptr;
}

void freeConsecNodes(u32 startIndex, int n) {
    if (freeListPop + n > FREE_LIST_SIZE) {
        DEBUG(1) fprintf(stderr, "Free list full!\n");
        return;
    }
    DEBUG(5) printf("adding %d nodes to free list...",n);
    freeListPop += n;
    while (n--) {
        freeList[firstFreeIndex++] = startIndex;
        firstFreeIndex &= FREE_LIST_SIZE_MASK;
        startIndex += 16;
    }
    DEBUG(5) printf(" new first free index is %d, free list pop: %d\n",firstFreeIndex,freeListPop);
}
