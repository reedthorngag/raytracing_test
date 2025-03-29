#include <glm/glm.hpp>
#include <stdio.h>
#include <stdlib.h>

#include "tetrahexa_tree.hpp"

int numNodes = 0;
Node* root;

void init() {
    root = new Node{0,{.branch = {}}};
}

u64 getBlock(Pos pos) {

    int posOffset = maxDepth * 2;

    int depth = 0;

    Node stack[maxDepth-1];

    stack[0] = root;

    while (depth < maxDepth) {
        posOffset -= 2;

        Pos curr = (pos >> posOffset) & 0b11;

        // not ideal, but it looks like it may be impossible to use bitshifts
        // to construct the index :(
        int index = curr.x + (4 * curr.y) + (16 * curr.z);

        if (stack[depth].flags & 1) return stack[depth].leaf.packedColor;

        if (!((stack[depth].branch.bitmap >> index) & 1)) return -1;

        // this might be used in future, its a tradeoff between memory and speed though
        //index = std::popcount(stack[depth].branch.bitmap << (64 - index));

        stack[depth+1] = stack[depth].branch.children[index];

        depth++; 
    }

    printf("Error: hit max depth without finding leaf node! (getBlock in tetrahexa_tree.cpp)\n");
    exit(1);

}

void putBlock(Pos pos, u64 color) {

    int posOffset = maxDepth * 2;

    int depth = 0;

    Node* stack[maxDepth-1];

    stack[0] = root;

    while (depth < maxDepth) {
        posOffset -= 2;

        Pos curr = (pos >> posOffset) & 0b11;

        // not ideal, but it looks like it may be impossible to use bitshifts
        // to construct the index :(
        int index = curr.x + (4 * curr.y) + (16 * curr.z);

        if (stack[depth]->flags & 1) {

            if (posOffset == 0) {
                stack[depth]->leaf.packedColor = color;
                return;
            }

            u32 leafFlags = stack[depth]->flags;
            u64 leafColor = stack[depth]->leaf.packedColor;

            Node** childPtrArray = new Node*[64];
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


        } else if (!((stack[depth]->branch.bitmap >> index) & 1)) {

            Node* child = new Node{};
            child->flags = 0;
            child->branch.children = new Node*[64];

            stack[depth]->branch.bitmap |= 1 << index;
            stack[depth]->branch.children[index] = child;

            stack[depth++] = child;
        }
    }

    printf("Error: hit max depth without finding leaf node! (putBlock in tetrahexa_tree.cpp)\n");
    exit(1);
}

