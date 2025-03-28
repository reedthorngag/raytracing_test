#include <glm/glm.hpp>

#include "tetrahexa_tree.hpp"

int numNodes = 0;
Node* root;

void init() {
    root = new Node{0,{.branch = {}}};
}

void putBlock(Pos pos, u64 color) {

    int posOffset = maxDepth * 2;

    Node* currNode = root;

    while (posOffset) {
        posOffset -= 2;

        Pos curr = (pos >> posOffset) & s;

        // not ideal, but it looks like it may be impossible to use bitshifts
        // to construct the index :(
        int index = curr.x + (4 * curr.y) + (16 * curr.z);
    }


}

