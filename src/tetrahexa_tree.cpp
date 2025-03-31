#include <glm/glm.hpp>
#include <stdio.h>
#include <stdlib.h>

#include "tetrahexa_tree.hpp"
#include "globals.hpp"

int numNodes = 0;
Node* root;

void init() {
    root = new Node{0,{.branch = {0,new Node*[64]{}}}};

    putBlock(Pos{1000,1000,1000}, 1234, 6);
    printf("block at 0,0,0 = %lld\n",getBlock({1000,1000,1000}));
}

u64 getBlock(Pos pos) {
    DEBUG printf("Getting block at {%d,%d,%d}...\n",pos.x,pos.y,pos.z);

    int posOffset = maxDepth * 2;

    int depth = 0;

    Node* stack[maxDepth];

    stack[0] = root;

    while (depth < maxDepth) {
        posOffset -= 2;

        Pos curr = (pos >> posOffset) & 0b11;

        // not ideal, but it looks like it may be impossible to use bitshifts
        // to construct the index :(
        int index = curr.x + (4 * curr.y) + (16 * curr.z);

        DEBUG printf("depth: %d, index: %d\n",depth, index);

        if (stack[depth]->flags & 1) {
            DEBUG printf("Found leaf at depth %d, returning color\n",depth);
            return stack[depth]->leaf.packedColor;
        }

        if (!((stack[depth]->branch.bitmap >> index) & 1)) {
            DEBUG printf("Found empty node at depth %d, index %d, returning -1\n",depth,index);
            return -1;
        }

        // this might be used in future, its a tradeoff between memory and speed though
        //index = std::popcount(stack[depth].branch.bitmap << (64 - index));

        stack[depth+1] = stack[depth]->branch.children[index];

        depth++; 
    }

    printf("Error: hit max depth without finding leaf node! (getBlock in tetrahexa_tree.cpp)\n");
    exit(1);

}

void deleteChildren(Node* node) {

    for (int i = 0; i < 64; i++) {
        if (node->branch.children[i]) {
            deleteChildren(node->branch.children[i]);
            delete node->branch.children[i];
        }
    }

    delete[] node->branch.children;
}

void putBlock(Pos pos, u64 color, int targetDepth) {
    DEBUG printf("Putting block at {%d,%d,%d} (size: %d) with color %lld...\n",pos.x,pos.y,pos.z,1<<((maxDepth-targetDepth)*2),color);
    targetDepth--; // convert to zero based index

    int posOffset = maxDepth * 2;

    int depth = 0;

    Node* stack[maxDepth];

    stack[0] = root;

    while (depth < maxDepth) {
        posOffset -= 2;

        Pos curr = (pos >> posOffset) & 0b11;

        // not ideal, but it looks like it may be impossible to use bitshifts
        // to construct the index :(
        int index = curr.x + (4 * curr.y) + (16 * curr.z);

        DEBUG printf("depth: %d, index: %d\n",depth, index);

        if (depth == targetDepth) {

            // if leaf
            if (stack[depth]->flags & 1) {
                DEBUG printf("Found leaf at target depth (%d), setting color...\n",depth);
                stack[depth]->leaf.packedColor = color;
                return;

            // else branch
            } else {
                DEBUG printf("Found branch at target depth (%d), converting to leaf...\n", depth);
                deleteChildren(stack[depth]);
                stack[depth]->leaf.packedColor = color;
                stack[depth]->flags = 1;
                return;
            }
        

        // if leaf
        } else if (stack[depth]->flags & 1) {

            u32 leafFlags = stack[depth]->flags;
            u64 leafColor = stack[depth]->leaf.packedColor;

            Node** childPtrArray = new Node*[64]{};
            Node* childNodes = new Node[64];

            for (int i = 0; i < 64; i++) {
                childPtrArray[i] = &childNodes[i];
                childNodes[i].flags = leafFlags;
                childNodes[i].leaf.packedColor = leafColor;
            }

            stack[depth]->flags = 0;
            stack[depth]->branch.bitmap = -1; // all set
            stack[depth]->branch.children = childPtrArray;

            stack[depth++] = childPtrArray[index];

        // else if air/empty
        } else if (!((stack[depth]->branch.bitmap >> index) & 1)) {

            DEBUG printf("Adding child node at depth %d, child index: %d\n",depth+1,index);

            Node* child = new Node{0,{.branch = {0, new Node*[64]{}}}};

            stack[depth]->branch.bitmap |= 1 << index;
            stack[depth]->branch.children[index] = child;

            stack[++depth] = child;

        } else {
            stack[depth+1] = stack[depth]->branch.children[index];
            depth++;
        }
    }

    printf("Error: hit max depth without finding leaf node! (putBlock in tetrahexa_tree.cpp)\n");
    exit(1);
}

