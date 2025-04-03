

#include "../types.hpp"
#include "types.hpp"


const int maxDepth = 6;

extern int numNodes;

extern Ptr root;

void init();

void traverseTree(Pos* pos, int count);

void putBlock(Pos pos, u64 color, int targetDepth);

void deleteChildren(Node* node);

u64 getBlock(Pos pos);




