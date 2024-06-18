#include <stdint.h>
#include <random>
#include <stdio.h>
#include <bit>

// 4^3 = 64
// uint64_t is 8 bytes
// 

//typedef uint64_t Node;

struct Pos;

struct Ray {
    double x;
    double y;
    double z;
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

// Pos pos = Pos{0,0,0};
// Ray vec = Ray{0.9,0.5,0};

// 0.9 / 0.5 = 1.8

// += 1 * 1.8

void nextIntersect(Pos* pos, Ray ray) {
    double ratio = (double)ray.y / (double)ray.x;
    
    double xDst = (pos->x+1) - pos->trueX;

    printf(" xDst: %lf ratio: %lf : %lf",xDst,ratio,xDst * ratio);
    if (pos->trueY + (xDst * ratio) > pos->y+1) {
        // next intercept is with x,y+1
        ratio = ray.x/ray.y;
        double yDst = ++pos->y - pos->trueY;
        pos->trueY = pos->y;
        pos->trueX += yDst*ratio;
    } else {
        // next intercept is with x+1,y
        printf(" here");
        pos->trueX = ++pos->x;
        pos->trueY += xDst * ratio;
    }
    printf("\n");
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
    Ray vec = Ray{.9,.5,1};

    while (pos.x < 10) {
        printf("pos: %d, %d  truePos: %lf %lf ",pos.x,pos.y,pos.trueX,pos.trueY);
        nextIntersect(&pos,vec);
        
    }

    return 0;
}







