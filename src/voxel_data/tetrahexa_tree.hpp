

#include "../types.hpp"
#include "types.hpp"

const int RGB_MASK = (1 << 21) - 1;
#define RGB_RANGE RGB_MASK
#define convertScale(x) ((u64)((float)x/255.0 * RGB_RANGE) & RGB_MASK)
#define RGB_TO_U64(r,g,b) ((convertScale(r) << 42) | (convertScale(g) << 21) | convertScale(b))

const int maxDepth = 6;

extern int numNodes;

extern Ptr root;

void initTetraHexaTree();

void traverseTree(Pos* pos, int count);

void putBlock(Pos pos, u64 color, int targetDepth);

void deleteChildren(Node* node);

u64 getBlock(Pos pos);




