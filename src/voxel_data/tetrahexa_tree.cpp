#include <glm/glm.hpp>
#include <stdio.h>
#include <stdlib.h>
#include <cstring>

#include "tetrahexa_tree.hpp"
#include "../globals.hpp"
#include "voxel_allocator.hpp"

int numNodes = 0;
Ptr root;

void initTetraHexaTree() {

    root = allocNode();
    Ptr array = allocConsecNodes((sizeof(u32) * 64) / 16);
    memset(array.ptr, 0, sizeof(u32) * 64);
    *root.ptr = {0,{.branch = {0, array.index}}};

    putBlock(Pos{1000,1000,1000}, 1, 5);
    putBlock(Pos{10,100,10}, 2, 6);
    putBlock(Pos{100,10,100}, 3, 6);
    putBlock(Pos{20,10,200}, 4, 5);
    putBlock(Pos{1,10,10}, 5, 6);
    putBlock(Pos{2,10,10}, 6, 6);
    putBlock(Pos{3,10,10}, 7, 6);
    putBlock(Pos{4,10,10}, 8, 6);

    printf("block at somewhere = %lld\n",getBlock({1002,1002,1003}));

    Pos* pos = new Pos[3]{
        {1,10,10},
        {2,10,10},
        {4,10,10}
    };

    traverseTree(pos, 3);

}

void traverseTree(Pos* pos, int count) {
    DEBUG(2) printf("running traverseTree on %d targets...\n",count);

    int posOffset;

    int depth = 0;

    Ptr stack[maxDepth];

    stack[0] = root;

    Pos target = *pos;

    Pos current = {};

    while (count--) {

        DEBUG(2) printf("finding (%d,%d,%d) from (%d,%d,%d)...\n",target.x,target.y,target.z,current.x,current.y,current.z);

        // find last common node and then jump to it and start search for the target

        int n = 0;
        int mask = 0b11 << ((maxDepth-1) * 2 - 2);
        while (depth && (target.x & mask) == (current.x & mask) &&
                (target.y & mask) == (current.y & mask) &&
                (target.z & mask) == (current.z & mask)) {
            n++;
            mask >>= 2;
        }

        if (n < depth) {
            DEBUG(3) printf("Retreating to depth %d from depth %d...\n",n,depth);
            depth = n;
        }

        posOffset = (maxDepth-1 - depth) * 2;

        while (depth < maxDepth) {
            posOffset -= 2;

            Pos curr = (target >> posOffset) & 0b11;

            int index = curr.z << 4 | curr.y << 2 | curr.x;

            DEBUG(3) printf("depth: %d, index: %d, pos: %d,%d,%d\n",depth, index, curr.x, curr.y, curr.z);

            if (stack[depth].ptr->flags & 1) {
                DEBUG(2) printf("Found leaf at depth %d, color: %lld\n\n",depth,stack[depth].ptr->leaf.packedColor);
                break;

            } else if (!((stack[depth].ptr->branch.bitmap >> index) & 1)) {
                DEBUG(2) printf("Found empty node at depth %d, index %d, returning -1\n",depth+1,index);
                break;
            }

            // this might be used in future, its a tradeoff between memory and speed though
            //index = std::popcount(stack[depth].ptr.branch.bitmap << (64 - index));

            stack[depth+1] = convertToPtr(((u32*)convertToPtr(stack[depth].ptr->branch.children).ptr)[index]);

            depth++;
        }

        current = target;
        target = *++pos;
    }
}

u64 getBlock(Pos pos) {
    DEBUG(2) printf("Getting block at (%d,%d,%d)...\n",pos.x,pos.y,pos.z);

    int posOffset = (maxDepth-1) * 2;

    int depth = 0;

    Ptr stack[maxDepth];

    stack[0] = root;

    while (depth < maxDepth) {
        posOffset -= 2;

        Pos curr = (pos >> posOffset) & 0b11;

        int index = curr.z << 4 | curr.y << 2 | curr.x;

        //DEBUG printf("depth: %d, index: %d\n",depth, index);
        DEBUG(3) printf("depth: %d, index: %d, pos: %d,%d,%d\n",depth, index, curr.x, curr.y, curr.z);

        if (stack[depth].ptr->flags & 1) {
            DEBUG(2) printf("Found leaf at depth %d, returning color\n",depth);
            return stack[depth].ptr->leaf.packedColor;

        } else if (!((stack[depth].ptr->branch.bitmap >> index) & 1)) {
            DEBUG(2) printf("Found empty node at depth %d, index %d, returning -1\n",depth+1,index);
            return -1;
        }

        // this might be used in future, its a tradeoff between memory and speed though
        //index = std::popcount(stack[depth].ptr.branch.bitmap << (64 - index));

        stack[depth+1] = convertToPtr(((u32*)convertToPtr(stack[depth].ptr->branch.children).ptr)[index]);

        depth++; 
    }

    printf("Error: hit max depth without finding leaf node! (getBlock in tetrahexa_tree.cpp)\n");
    exit(1);

}

void deleteChildren(Ptr node) {
    if (node.ptr->flags & 1)
        return;

    u32* children = (u32*)convertToPtr(node.ptr->branch.children).ptr;
    for (int i = 0; i < 64; i++) {
        if (children[i]) {
            deleteChildren(convertToPtr(children[i]));
            freeNode(children[i]);
        }
    }

    freeConsecNodes(node.ptr->branch.children, (sizeof(u32) * 64) / 16);
}

void putBlock(Pos pos, u64 color, int targetDepth) {
    DEBUG(3) printf("Putting block at (%d,%d,%d) (size: %d) with color %lld...\n",pos.x,pos.y,pos.z,1<<((maxDepth-targetDepth)*2),color);
    targetDepth--; // convert to zero based index

    int posOffset = (maxDepth-1) * 2;

    int depth = 0;

    Ptr stack[maxDepth];

    stack[0] = root;

    while (depth < maxDepth) {
        posOffset -= 2;

        Pos curr = (pos >> posOffset) & 0b11;

        int index = curr.z << 4 | curr.y << 2 | curr.x;

        DEBUG(4) printf("depth: %d, index: %d\n",depth, index);
 
        if (depth == targetDepth) {

            // if leaf
            if (stack[depth].ptr->flags & 1) {
                DEBUG(3) printf("Found leaf at target depth %d, setting color...\n",depth);
                stack[depth].ptr->leaf.packedColor = color;
                return;

            // else branch
            } else {
                DEBUG(3) printf("Found branch at target depth %d, converting to leaf...\n", depth);
                deleteChildren(stack[depth]);
                stack[depth].ptr->leaf.packedColor = color;
                stack[depth].ptr->flags = 1;
                return;
            }
        

        // if leaf
        } else if (stack[depth].ptr->flags & 1) {

            DEBUG(4) printf("Turning leaf into branch at depth %d and adding child, child index: %d\n",depth,index);

            u32 leafFlags = stack[depth].ptr->flags;
            u64 leafColor = stack[depth].ptr->leaf.packedColor;

            Ptr childPtrArray = allocConsecNodes(sizeof(u32) * 64);
            memset(childPtrArray.ptr, 0, sizeof(u32) * 64);

            Ptr childNodes = allocConsecNodes(64 * sizeof(Node));
            memset(childNodes.ptr, 0, sizeof(Node) * 64);

            for (int i = 0; i < 64; i++) {
                ((u32*)childPtrArray.ptr)[i] = childNodes.index + (i << 4);
                childNodes.ptr[i].flags = leafFlags;
                childNodes.ptr[i].leaf.packedColor = leafColor;
            }

            stack[depth].ptr->flags = 0;
            stack[depth].ptr->branch.bitmap = -1; // all set
            stack[depth].ptr->branch.children = childPtrArray.index;

            stack[++depth] = Ptr{((u32*)childPtrArray.ptr)[index], &childNodes.ptr[index]};

        // else if air/empty
        } else if (!((stack[depth].ptr->branch.bitmap >> index) & 1)) {

            DEBUG(4) printf("Adding child node at depth %d, child index: %d\n",depth+1,index);

            Ptr child = allocNode();
            Ptr array = allocConsecNodes(sizeof(u32) * 64);
            memset(array.ptr, 0, sizeof(u32) * 64);
            *child.ptr = {0,{.branch = {0, array.index}}};
            DEBUG(5) printf("child: %d, array: %d\n",child.index,array.index);

            stack[depth].ptr->branch.bitmap |= 1ull << index;
            ((u32*)convertToPtr(stack[depth].ptr->branch.children).ptr)[index] = child.index;
            

            stack[++depth] = child;

        } else {
            stack[depth+1] = convertToPtr(((u32*)convertToPtr(stack[depth].ptr->branch.children).ptr)[index]);
            depth++;
        }
    }

    printf("Error: hit max depth without finding leaf node! (putBlock in tetrahexa_tree.cpp)\n");
    exit(1);
}

