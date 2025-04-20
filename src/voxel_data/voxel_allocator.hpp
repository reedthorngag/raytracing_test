#include <stdlib.h>
#include <stdio.h>
#include <memoryapi.h>
#include <errhandlingapi.h>
#include <windows.h>
#include <GL/glew.h>

#include "../globals.hpp"
#include "../types.hpp"
#include "types.hpp"

const int BLOCK_SIZE_BITS = 22;
const int MAX_BLOCKS = 1 << (32-BLOCK_SIZE_BITS); // 1024
const int BLOCK_SIZE = 1 << BLOCK_SIZE_BITS; // 4194304
const int BLOCK_SIZE_MASK = BLOCK_SIZE - 1;

extern Block nodeBlocks[]; // 1024
extern Block arrayBlocks[]; // 1024

const int FREE_LIST_SIZE = 1 << 12; // 4096
const int FREE_LIST_SIZE_MASK = FREE_LIST_SIZE - 1;
extern u32 nodeFreeList[];
extern u32 arrayFreeList[];

extern u32 nodeNextAllocIndex;
extern int nodeFreeListPop;
extern int nodeNextFreeListIndex;
extern int nodeFirstFreeIndex;

extern u32 arrayNextAllocIndex;
extern int arrayFreeListPop;
extern int arrayNextFreeListIndex;
extern int arrayFirstFreeIndex;

extern GLuint nodeSsbo;
extern u32 currentNodeSsboSize;
extern GLuint arraySsbo;
extern u32 currentArraySsboSize;

inline void updateSsboData() {

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, arraySsbo);

    u32 n = 1;
    for (; n < MAX_BLOCKS && arrayBlocks[n].ptr; n++);
    
    if (n != currentArraySsboSize) {
        currentArraySsboSize = n;
        glBufferData(GL_SHADER_STORAGE_BUFFER, n << BLOCK_SIZE_BITS, NULL, GL_DYNAMIC_DRAW);
    }

    for (u32 i = 0; i < n; i++)
        if (arrayBlocks[i].modified) {
            arrayBlocks[i].modified = false;
            printf("Uploading array block %d to gpu!\n",n);
            glBufferSubData(GL_SHADER_STORAGE_BUFFER, i << BLOCK_SIZE_BITS, BLOCK_SIZE, arrayBlocks[i].ptr);
            DEBUG(2) checkGlError("Upload array data");
        }


    glBindBuffer(GL_SHADER_STORAGE_BUFFER, nodeSsbo);

    n = 1;
    for (; n < MAX_BLOCKS && nodeBlocks[n].ptr; n++);
    
    if (n != currentNodeSsboSize) {
        currentNodeSsboSize = n;
        glBufferData(GL_SHADER_STORAGE_BUFFER, n << BLOCK_SIZE_BITS, NULL, GL_DYNAMIC_DRAW);
    }

    for (u32 i = 0; i < n; i++)
        if (nodeBlocks[i].modified) {
            nodeBlocks[i].modified = false;
            printf("Uploading node block %d to gpu!\n",n);
            glBufferSubData(GL_SHADER_STORAGE_BUFFER, i << BLOCK_SIZE_BITS, BLOCK_SIZE, nodeBlocks[i].ptr);
            DEBUG(2) checkGlError("Upload node data");
        }

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

inline void initVoxelDataAllocator() {

    glGenBuffers(1, &arraySsbo);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, arraySsbo);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, arraySsbo);

    glGenBuffers(1, &nodeSsbo);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, nodeSsbo);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, nodeSsbo);
}

inline Ptr convertToPtr(u32 index) {
    return {index,(Node*)(nodeBlocks[(index<<4) >> BLOCK_SIZE_BITS].ptr + ((index<<4) & BLOCK_SIZE_MASK)),&nodeBlocks[(index<<4) >> BLOCK_SIZE_BITS]};
}

inline Ptr convertToArrayPtr(u32 index) {
    return {index,(Node*)(arrayBlocks[(index<<8) >> BLOCK_SIZE_BITS].ptr + ((index<<8) & BLOCK_SIZE_MASK)),&arrayBlocks[(index<<8) >> BLOCK_SIZE_BITS]};
}

inline void mallocNodeBlock(int n) {
    nodeBlocks[n] = Block{(u8*)malloc(BLOCK_SIZE),false};
}

inline void mallocArrayBlock(int n) {
    arrayBlocks[n] = Block{(u8*)malloc(BLOCK_SIZE),false};
}

Ptr allocNode();
Ptr allocArray();

// number of bytes to alloc (must be multiple of 16)
Ptr allocConsecNodes(int n);

inline void freeNode(u32 index) {
    if (nodeFreeListPop >= 1024) {
        DEBUG(1) fprintf(stderr, "Free list is full!\n");
        return;
    }

    nodeFreeList[nodeFirstFreeIndex++] = index;
    nodeFirstFreeIndex &= FREE_LIST_SIZE_MASK; // same as % 1024 but faster
    nodeFreeListPop++;
}

inline void freeArray(u32 index) {
    if (arrayFreeListPop >= 1024) {
        DEBUG(1) fprintf(stderr, "Array Free list is full!\n");
        return;
    }

    arrayFreeList[arrayFirstFreeIndex++] = index;
    arrayFirstFreeIndex &= FREE_LIST_SIZE_MASK; // same as % 1024 but faster
    arrayFreeListPop++;
}

void freeConsecNodes(u32 startIndex, int n);

