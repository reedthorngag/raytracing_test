#include <stdlib.h>
#include <stdio.h>
#include <memoryapi.h>
#include <errhandlingapi.h>
#include <windows.h>

#include "../globals.hpp"
#include "../types.hpp"
#include "types.hpp"

const int BLOCK_SIZE_BITS = 22;
const int BLOCK_SIZE = 1 << BLOCK_SIZE_BITS; // 4194304
const int BLOCK_SIZE_MASK = BLOCK_SIZE - 1;

extern u8* blocks[]; // 1024

const int FREE_LIST_SIZE = 1 << 12; // 4096
const int FREE_LIST_SIZE_MASK = FREE_LIST_SIZE - 1;
extern u32 freeList[];

extern u32 nextAllocIndex;

extern int freeListPop;
extern int nextFreeListIndex;
extern int firstFreeIndex;

inline Ptr convertToPtr(u32 index) {
    return {index,(Node*)(blocks[index >> BLOCK_SIZE_BITS] + (index & BLOCK_SIZE_MASK))};
}

inline void mallocBlock(int n) {
    blocks[n] = (u8*)malloc(BLOCK_SIZE);
}

Ptr allocNode();

// number of bytes to alloc (must be multiple of 16)
Ptr allocConsecNodes(int n);

inline void freeNode(u32 index) {
    if (freeListPop >= 1024) {
        DEBUG(1) fprintf(stderr, "Free list is full!\n");
        return;
    }

    freeList[firstFreeIndex++] = index;
    firstFreeIndex &= FREE_LIST_SIZE_MASK; // same as % 1024 but faster
    freeListPop++;
}

void freeConsecNodes(u32 startIndex, int n);

