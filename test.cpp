#include <stdio.h>
#include <cstdint>
#include <cmath>

#include "./src/chunk.hpp"

int width = 800;
int halfWidth = width/2;
int height = 600;
int halfHeight = height/2;

struct Ray {
    double x;
    double y;
    double z;

    int stepX;
    int stepY;
    int stepZ;

    double ratioYtoX;
    double ratioYtoZ;
    double ratioXtoY;
    double ratioXtoZ;
    double ratioZtoX;
    double ratioZtoY;
};

double ifZeroMakeOne(double n) {
    if (n == 0) {
        return 1;
    }
    return n;
}

Ray buildRay(double x, double y, double z) {
    Ray ray;

    ray.x = x;
    ray.y = y;
    ray.z = z;

    ray.stepX = 0;
    ray.stepY = 0;
    ray.stepZ = 0;

    if (x < 0)
        ray.stepX = -1;
    else if (x > 0)
        ray.stepX = 1;
    if (y < 0)
        ray.stepY = -1;
    else if (y > 0)
        ray.stepY = 1;
    if (z < 0)
        ray.stepZ = -1;
    else if (z > 0)
        ray.stepZ = 1;

    ray.ratioYtoX = abs(y) / abs(ifZeroMakeOne(x));
    ray.ratioYtoZ = abs(y) / abs(ifZeroMakeOne(z));
    ray.ratioXtoY = abs(x) / abs(ifZeroMakeOne(y));
    ray.ratioXtoZ = abs(x) / abs(ifZeroMakeOne(z));
    ray.ratioZtoX = abs(z) / abs(ifZeroMakeOne(x));
    ray.ratioZtoY = abs(z) / abs(ifZeroMakeOne(y));

    return ray;
}

struct Pos {
    double x;
    double y;
    double z;
    double trueX;
    double trueY;
    double trueZ;
};

Pos pos;
Ray ray;

void nextIntersect() {

    double xDst = (pos.x + ray.stepX) - pos.trueX;
    double yDst = (pos.y + ray.stepY) - pos.trueY;
    double zDst = (pos.z + ray.stepZ) - pos.trueZ;

    if (pos.trueY + (xDst * ray.ratioYtoX) > pos.y + ray.stepY) {
        // next intercept is with x,y+1

        if (pos.trueZ + (yDst * ray.ratioZtoY) > pos.z + ray.stepZ) {
            pos.trueZ = (pos.z += ray.stepZ);
            pos.trueX += zDst * ray.ratioXtoZ;
            pos.trueY += zDst * ray.ratioYtoZ;
        } else {
            pos.trueY = (pos.y += ray.stepY);
            pos.trueX += yDst * ray.ratioXtoY;
            pos.trueZ += yDst * ray.ratioZtoY;
        }
    } else {
        // next intercept is with x+1,y
        
        if (pos.trueZ + (xDst * ray.ratioZtoX) > pos.z + ray.stepZ) {
            pos.trueZ = (pos.z += ray.stepZ);
            pos.trueX += zDst * ray.ratioXtoZ;
            pos.trueY += zDst * ray.ratioYtoZ;
        } else {
            pos.trueX = (pos.x += ray.stepX);
            pos.trueY += xDst * ray.ratioYtoX;
            pos.trueZ += xDst * ray.ratioZtoX;
            printf("here\n");
        }
    }

    pos.x = floor(pos.trueX + 0.00000001);
    pos.y = floor(pos.trueY + 0.00000001);
    pos.z = floor(pos.trueZ + 0.00000001);
}

int main() {

    ray = buildRay(0,0,1);

    double x = 0;
    double y = 50;
    double z = 0;

    pos.x = x;
    pos.y = y;
    pos.z = z;
    pos.trueX = x;
    pos.trueY = y;
    pos.trueZ = z;

    printf("%d,%d,%d\n",ray.stepX,ray.stepY,ray.stepZ);

    nextIntersect();

    printf("%lf %lf %lf\n%lf %lf %lf\n\n",pos.x,pos.y,pos.z,pos.trueX,pos.trueY,pos.trueZ);
}

