

#include "../types.hpp"
#include "types.hpp"

const int maxDepth = 6;

extern int numNodes;

extern Ptr root;

void initTetraHexaTree();

void traverseTree(Pos* pos, int count);

void putBlock(Pos pos, u64 color, u32 properties, int targetDepth);

void deleteChildren(Ptr node);

u64 getBlock(Pos pos);




