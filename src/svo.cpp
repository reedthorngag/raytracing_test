#include <stdint.h>
#include <random>
#include <stdio.h>
#include <bit>

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
    Pos& operator +=(Ray a) {
        

        return *this;
    }
};

void nextIntersect(Pos* pos, Ray ray) {
    
    double xDst = (pos->x + 1) - pos->trueX;
    double yDst = (pos->y + 1) - pos->trueY;
    double zDst = (pos->z + 1) - pos->trueZ;


    if (pos->trueY + (xDst * ray.ratioYtoX) > pos->y+1) {
        // next intercept is with x,y+1

        if (pos->trueZ + (yDst * ray.ratioZtoY) > pos->z+1) {
            debug printf(" 1");
            pos->trueZ = ++pos->z;
            pos->trueX += zDst * ray.ratioXtoZ;
            pos->trueY += zDst * ray.ratioYtoZ;
        } else {
            debug printf(" 2");
            pos->trueY = ++pos->y;
            pos->trueX += yDst * ray.ratioXtoY;
            pos->trueZ += yDst * ray.ratioZtoY;
        }
    } else {
        // next intercept is with x+1,y
        
        if (pos->trueZ + (xDst * ray.ratioZtoX) > pos->z+1) {
            debug printf(" 3");
            pos->trueZ = ++pos->z;
            pos->trueX += zDst * ray.ratioXtoZ;
            pos->trueY += zDst * ray.ratioYtoZ;
        } else {
            debug  printf(" 4");
            pos->trueX = ++pos->x;
            pos->trueY += xDst * ray.ratioYtoX;
            pos->trueZ += xDst * ray.ratioZtoX;
        }
    }

    //printf(" : %d,%d,%d",pos->x==pos->trueX,pos->y==pos->trueY,pos->z==pos->trueZ);

    pos->x = floor(pos->trueX + 0.00000001);
    pos->y = floor(pos->trueY + 0.00000001);
    pos->z = floor(pos->trueZ + 0.00000001);
    debug printf("\n");
}

struct Leaf {
    bool solid;
};

struct Node {
    Pos origin;
    Pos size;
    uint64_t bitmap;
    Leaf* children;
};

int main() {

    printf("\033c"); // resets the terminal

    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> range(0,1);


    uint64_t bitmap;
    for (int i = 0; i < 64; i++)
        if (range(rng))
            bitmap |= (1 << i);

    Node node = Node{{0,0,0},{4,4,4},bitmap};

    Leaf* leafs = new Leaf[std::popcount(bitmap)]{};

    for (int i = 0; i < std::popcount(bitmap); i++) {
        leafs[i].solid = true;
    }

    Pos pos = Pos{1,1,1,1,1,1};
    Ray vec = Ray(.9,.5,.3);

    while (pos.x < 20) {
        debug printf("pos: %d, %d, %d  truePos: %lf, %lf, %lf ",pos.x,pos.y,pos.z,pos.trueX,pos.trueY,pos.trueZ);
        nextIntersect(&pos,vec);
        
    }
    debug printf("pos: %d, %d, %d  truePos: %lf, %lf, %lf ",pos.x,pos.y,pos.z,pos.trueX,pos.trueY,pos.trueZ);

    return 0;
}







