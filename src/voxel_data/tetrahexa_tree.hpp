
#include "../types.hpp"
#include "../globals.hpp"
#include "types.hpp"

const int maxDepth = 6;

extern int numNodes;

extern Ptr root;

void initTetraHexaTree();

void traverseTree(Pos* pos, int count);

void putBlock(Pos pos, Block block, int targetDepth);

void deleteChildren(Ptr node);

Block getBlock(Pos pos);

Block deleteBlock(Pos pos, int level);




