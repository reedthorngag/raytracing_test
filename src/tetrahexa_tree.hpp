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
            Node** children;
        } branch;
        struct {
            u64 packedColor;
            u64 spare; // u32 on gpu
        } leaf;
    };
};

extern int numNodes;

extern Node* root;

void init();

void putBlock(Pos pos, u64 color, int targetDepth);

void deleteChildren(Node* node);

u64 getBlock(Pos pos);




