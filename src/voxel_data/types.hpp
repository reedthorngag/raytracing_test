
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

struct __attribute__((packed)) Branch {
    u64 bitmap;
    u32 flags;
    u32 children;
};

struct __attribute__((packed)) Leaf {
    u64 packedColor;
    u32 flags;
    float metadata;
};

// lowest bit in flags is set in leaf
struct Node {
    union {
        Branch branch;
        Leaf leaf;
    };
} __attribute__((packed));

struct BlockPtr {
    u8* ptr;
    bool modified;
};

struct Ptr {
    u32 index;
    Node* ptr;
    BlockPtr* block;
};

#endif

