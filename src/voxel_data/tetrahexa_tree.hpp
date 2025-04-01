

#include "../types.hpp"


const int maxDepth = 6;

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
    u32 flags;
    union {
        struct {
            u64 bitmap;
            u32 children;
        } branch;
        struct {
            u64 packedColor;
            u32 spare;
        } leaf;
    };
};

extern int numNodes;

extern u32 root;

void init();

void traverseTree(Pos* pos, int count);

void putBlock(Pos pos, u64 color, int targetDepth);

void deleteChildren(Node* node);

u64 getBlock(Pos pos);




