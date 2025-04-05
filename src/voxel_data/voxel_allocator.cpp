
#include "voxel_allocator.hpp"


Block blocks[MAX_BLOCKS]{};

u32 freeList[FREE_LIST_SIZE_MASK];

u32 nextAllocIndex = 0;

int freeListPop = 0;
int nextFreeListIndex = 0;
int firstFreeIndex = 0;

GLuint ssbo;
u32 currentSsboSize = 0;

Ptr allocNode() {
    if (freeListPop--) {
        u32 node = freeList[nextFreeListIndex++];
        nextFreeListIndex &= FREE_LIST_SIZE_MASK;
        blocks[node >> BLOCK_SIZE_BITS].modified = true;

        DEBUG(5) printf("node found in free list: %u next free list index: %d\n",node,nextFreeListIndex);

        return convertToPtr(node);
    }
    freeListPop++;

    if (!blocks[nextAllocIndex >> BLOCK_SIZE_BITS].ptr) {
        mallocBlock(nextAllocIndex >> BLOCK_SIZE_BITS);
    }

    blocks[nextAllocIndex >> BLOCK_SIZE_BITS].modified = true;
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

    blocks[nextAllocIndex >> BLOCK_SIZE_BITS].modified = true;
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
