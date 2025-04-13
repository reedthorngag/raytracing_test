
#include "voxel_allocator.hpp"


Block nodeBlocks[MAX_BLOCKS]{};
Block arrayBlocks[MAX_BLOCKS]{};

u32 nodeFreeList[FREE_LIST_SIZE_MASK];
u32 arrayFreeList[FREE_LIST_SIZE_MASK];

u32 nodeNextAllocIndex = 0;
int nodeFreeListPop = 0;
int nodeNextFreeListIndex = 0;
int nodeFirstFreeIndex = 0;

u32 arrayNextAllocIndex = 0;
int arrayFreeListPop = 0;
int arrayNextFreeListIndex = 0;
int arrayFirstFreeIndex = 0;

GLuint nodeSsbo;
u32 currentNodeSsboSize = 0;
GLuint arraySsbo;
u32 currentArraySsboSize = 0;

Ptr allocNode() {
    if (nodeFreeListPop--) {
        u32 node = nodeFreeList[nodeNextFreeListIndex++];
        nodeNextFreeListIndex &= FREE_LIST_SIZE_MASK;
        nodeBlocks[node >> (BLOCK_SIZE_BITS-4)].modified = true;

        DEBUG(5) printf("node found in free list: %u next free list index: %d\n",node,nodeNextFreeListIndex);

        return convertToPtr(node);
    }
    nodeFreeListPop++;

    if (!nodeBlocks[nodeNextAllocIndex >> BLOCK_SIZE_BITS].ptr) {
        mallocNodeBlock(nodeNextAllocIndex >> BLOCK_SIZE_BITS);
    }

    nodeBlocks[nodeNextAllocIndex >> BLOCK_SIZE_BITS].modified = true;
    Ptr ptr = convertToPtr(nodeNextAllocIndex >> 4);
    nodeNextAllocIndex += 16;
    return ptr;
}

Ptr allocArray() {
    if (arrayFreeListPop--) {
        u32 array = arrayFreeList[arrayNextFreeListIndex++];
        arrayNextFreeListIndex &= FREE_LIST_SIZE_MASK;
        arrayBlocks[array >> (BLOCK_SIZE_BITS-8)].modified = true;

        DEBUG(5) printf("array found in free list: %u next free list index: %d\n",array,arrayNextFreeListIndex);

        return convertToPtr(array);
    }
    arrayFreeListPop++;

    if (!arrayBlocks[arrayNextAllocIndex >> BLOCK_SIZE_BITS].ptr) {
        mallocArrayBlock(arrayNextAllocIndex >> BLOCK_SIZE_BITS);
    }

    arrayBlocks[arrayNextAllocIndex >> BLOCK_SIZE_BITS].modified = true;
    Ptr ptr = convertToArrayPtr(arrayNextAllocIndex >> 8);
    arrayNextAllocIndex += 256;
    return ptr;
}

Ptr allocConsecNodes(int n) {

    if ((nodeNextAllocIndex & BLOCK_SIZE_MASK) + (n << 4) > BLOCK_SIZE) {

        freeConsecNodes(nodeNextAllocIndex >> 4, n);
        nodeNextAllocIndex += n << 4;

        mallocNodeBlock(nodeNextAllocIndex >> BLOCK_SIZE_BITS);
    }

    nodeBlocks[nodeNextAllocIndex >> BLOCK_SIZE_BITS].modified = true;
    Ptr ptr = convertToPtr(nodeNextAllocIndex >> 4);
    nodeNextAllocIndex += n << 4;
    return ptr;
}

void freeConsecNodes(u32 startIndex, int n) {
    if (nodeFreeListPop + n > FREE_LIST_SIZE) {
        DEBUG(1) fprintf(stderr, "Free list full!\n");
        return;
    }
    DEBUG(5) printf("adding %d nodes to free list...",n);
    nodeFreeListPop += n;
    while (n--) {
        nodeFreeList[nodeFirstFreeIndex++] = startIndex++;
        nodeFirstFreeIndex &= FREE_LIST_SIZE_MASK;
    }
    DEBUG(5) printf(" new first free index is %d, free list pop: %d\n",nodeFirstFreeIndex,nodeFreeListPop);
}
