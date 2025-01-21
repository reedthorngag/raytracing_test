#include <stdint.h>
#include <random>
#include <stdio.h>
#include <bit>
#include <vector>
#include <chrono>

long curr_time() { return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count(); }

#define DEBUG 0

#if DEBUG
#define debug
#else
#define debug if (false)
#endif

struct Pos;

struct Ray {
    double x;
    double y;
    double z;

    double ratioYtoX;
    double ratioYtoZ;
    double ratioXtoY;
    double ratioXtoZ;
    double ratioZtoX;
    double ratioZtoY;

    Ray(double x, double y, double z) {
        this->x = x;
        this->y = y;
        this->z = z;

        this->ratioYtoX = (double)y / (double)x;
        this->ratioYtoZ = (double)y / (double)z;
        this->ratioXtoY = (double)x / (double)y;
        this->ratioXtoZ = (double)x / (double)z;
        this->ratioZtoX = (double)z / (double)x;
        this->ratioZtoY = (double)z / (double)y;
    }
};

struct Pos {
    int x;
    int y;
    int z;
    double trueX;
    double trueY;
    double trueZ;

    Pos operator +(Pos pos) const {
        return Pos(this->x+pos.x,this->y+pos.y,this->z+pos.z);
    }

    Pos operator +(int i) const {
        return Pos(this->x+i,this->y+i,this->z+i);
    }

    bool operator <(int i) const {
        return this->x < i || this->y < i || this->z < i;
    }

    Pos operator -(Pos pos) const {
        return Pos(this->x-pos.x,this->y-pos.y,this->z-pos.z);
    }

    Pos(int x, int y, int z) {
        this->x = x;
        this->trueX = x;
        this->y = y;
        this->trueY = y;
        this->z = z;
        this->trueZ = z;
    }

    Pos() {
        this->x = 0;
        this->trueX = 0;
        this->y = 0;
        this->trueY = 0;
        this->z = 0;
        this->trueZ = 0;
    }
};

// TODO: rewrite without branching
void nextIntersect(Pos* pos, Ray ray, int step) {
    
    double xDst = (pos->x + step) - pos->trueX;
    double yDst = (pos->y + step) - pos->trueY;
    double zDst = (pos->z + step) - pos->trueZ;


    if (pos->trueY + (xDst * ray.ratioYtoX) > pos->y + step) {
        // next intercept is with x,y+1

        if (pos->trueZ + (yDst * ray.ratioZtoY) > pos->z + step) {
            debug printf(" 1");
            pos->trueZ = (pos->z += step);
            pos->trueX += zDst * ray.ratioXtoZ;
            pos->trueY += zDst * ray.ratioYtoZ;
        } else {
            debug printf(" 2");
            pos->trueY = (pos->y += step);
            pos->trueX += yDst * ray.ratioXtoY;
            pos->trueZ += yDst * ray.ratioZtoY;
        }
    } else {
        // next intercept is with x+1,y
        
        if (pos->trueZ + (xDst * ray.ratioZtoX) > pos->z + step) {
            debug printf(" 3");
            pos->trueZ = (pos->z += step);
            pos->trueX += zDst * ray.ratioXtoZ;
            pos->trueY += zDst * ray.ratioYtoZ;
        } else {
            debug  printf(" 4");
            pos->trueX = (pos->x += step);
            pos->trueY += xDst * ray.ratioYtoX;
            pos->trueZ += xDst * ray.ratioZtoX;
        }
    }

    pos->x = floor(pos->trueX + 0.00000001);
    pos->y = floor(pos->trueY + 0.00000001);
    pos->z = floor(pos->trueZ + 0.00000001);
    debug printf("\n");
}


// Nodes are stored in groups of 8, 1 group should be 32 bytes (max) (so it fits in the smallest size cache line)

// child offset requirements:
// at tree max height, the offset is a maximum of 8 * node size
// at tree max - 1 it is a maximum of 8 * 8 * node size
// etc


/* Node gpu structure: (1 bytes)
 *
 * uint8_t flags - if bit 1 set, then leaf node, and offset is encoded color value
 * uint8_t bitmap
 * uint32_t offset - cpu side, Node[8]*
 * 
*/

struct Node {
    uint8_t flags;
    uint8_t bitmap;
    uint32_t offset;
};

int main1() {

    printf("\033cHello world!"); // resets the terminal

    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> range(0,1);


    uint64_t bitmap;
    for (int i = 0; i < 64; i++)
        if (range(rng))
            bitmap |= (1 << i);

    Pos pos = Pos(1,1,1);
    Ray vec = Ray(.9,.5,.3);

    while (pos.x < 20) {
        debug printf("pos: %d, %d, %d  truePos: %lf, %lf, %lf ",pos.x,pos.y,pos.z,pos.trueX,pos.trueY,pos.trueZ);
        nextIntersect(&pos,vec,4);
        
    }
    debug printf("pos: %d, %d, %d  truePos: %lf, %lf, %lf ",pos.x,pos.y,pos.z,pos.trueX,pos.trueY,pos.trueZ);

    return 0;
}







