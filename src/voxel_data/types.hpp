
#include "../types.hpp"

#ifndef VOXEL_DATA_TYPES
#define VOXEL_DATA_TYPES

struct Pos {
    int x;
    int y;
    int z;

    Pos operator >>(int n) {
        return Pos{
            x >> n,
            y >> n,
            z >> n
        };
    }

    Pos operator &(int n) {
        return Pos{
            x & n,
            y & n,
            z & n
        };
    }
};


// lowest bit in flags is set in leaf
struct Node {
    union {
        struct __attribute__((packed)) {
            u64 bitmap;
            u32 flags;
            u32 children;
        } branch;
        struct __attribute__((packed)) {
            u64 packedColor;
            u32 flags;
            u32 spare;
        } leaf;
    };
} __attribute__((packed));

struct Block {
    u8* ptr;
    bool modified;
};

struct Ptr {
    u32 index;
    Node* ptr;
    Block* block;
};

#endif

