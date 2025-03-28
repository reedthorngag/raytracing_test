#include <stdint.h>

#define u64 uint64_t
#define u32 uint32_t
#define u16 uint16_t
#define u8 uint8_t

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
        }
    }

    Pos operator &(int n) {
        return Pos{
            x & n,
            y & n,
            z & n
        }
    }
};

struct Branch {
    u64 bitmap;
    Node* children;
};

struct Leaf {
    u64 packed_color;
    u32 spare;
};

// lowest bit in flags is set in leaf
struct Node {
    u32 flags;
    union {
        branch branch;
        leaf leaf;
    };
};

extern int numNodes;

extern Node* root;





