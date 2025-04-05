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

struct Block {
    u8* ptr;
    bool modified;
};

extern Block blocks[]; // 1024

const int FREE_LIST_SIZE = 1 << 12; // 4096
const int FREE_LIST_SIZE_MASK = FREE_LIST_SIZE - 1;
extern u32 freeList[];

extern u32 nextAllocIndex;

extern int freeListPop;
extern int nextFreeListIndex;
extern int firstFreeIndex;

extern GLuint ssbo;
extern u32 currentSsboSize;

inline void updateSsboData() {

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);

    u32 n = 1;
    for (; n < MAX_BLOCKS && blocks[n].ptr; n++);
    
    if (n != currentSsboSize) {
        currentSsboSize = n;
        glBufferData(GL_SHADER_STORAGE_BUFFER, n << BLOCK_SIZE_BITS, NULL ,GL_DYNAMIC_DRAW);
    }

    for (u32 i = 0; i < n; i++)
        if (blocks[i].modified)
            glBufferSubData(GL_SHADER_STORAGE_BUFFER, i << BLOCK_SIZE_BITS, BLOCK_SIZE, blocks[i].ptr);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

inline void initVoxelDataAllocator() {

    glGenBuffers(1, &ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);

    updateSsboData();

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, ssbo); // bind to layout = 3
}

inline Ptr convertToPtr(u32 index) {
    return {index,(Node*)(blocks[index >> BLOCK_SIZE_BITS].ptr + (index & BLOCK_SIZE_MASK))};
}

inline void mallocBlock(int n) {
    blocks[n] = Block{(u8*)malloc(BLOCK_SIZE),false};
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

