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

double matchSign(double a, double sign) {
    if ((a < 0 && sign < 0) || (a > 0 && sign > 0)) return a;
    return -a;
}

Ray buildRay(double x, double y, double z) {
    Ray ray;

    ray.x = x;
    ray.y = y;
    ray.z = z;

    ray.stepX = 1;
    ray.stepY = 1;
    ray.stepZ = 1;

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

    ray.ratioYtoX = matchSign(abs(y) / abs(ifZeroMakeOne(x)),ray.stepY);
    ray.ratioYtoZ = matchSign(abs(y) / abs(ifZeroMakeOne(z)),ray.stepY);
    ray.ratioXtoY = matchSign(abs(x) / abs(ifZeroMakeOne(y)),ray.stepX);
    ray.ratioXtoZ = matchSign(abs(x) / abs(ifZeroMakeOne(z)),ray.stepX);
    ray.ratioZtoX = matchSign(abs(z) / abs(ifZeroMakeOne(x)),ray.stepZ);
    ray.ratioZtoY = matchSign(abs(z) / abs(ifZeroMakeOne(y)),ray.stepZ);

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

void nextIntersect(bool silent) {

    double xDst = abs((pos.x + ray.stepX) - pos.trueX);
    double yDst = abs((pos.y + ray.stepY) - pos.trueY);
    double zDst = abs((pos.z + ray.stepZ) - pos.trueZ);

    if (!silent) printf("Dsts: %lf, %lf, %lf\n",xDst,yDst,zDst);

    if (abs(pos.trueY + (xDst * ray.ratioYtoX)) >= abs(pos.y + ray.stepY)) {
        // next intercept is with x,y+1

        if (abs(pos.trueZ + (yDst * ray.ratioZtoY)) >= abs(pos.z + ray.stepZ)) {
            pos.trueZ = (pos.z += ray.stepZ);
            pos.trueX += zDst * ray.ratioXtoZ;
            pos.trueY += zDst * ray.ratioYtoZ;
            if (!silent) printf("Z, Y branch\n");
        } else {
            pos.trueY = (pos.y += ray.stepY);
            pos.trueX += yDst * ray.ratioXtoY;
            pos.trueZ += yDst * ray.ratioZtoY;
            if (!silent) printf("Y\n");
        }
    } else {
        // next intercept is with x+1,y
        
        if (abs(pos.trueZ + (yDst * ray.ratioZtoX)) >= abs(pos.z + ray.stepZ)) {
            pos.trueZ = (pos.z += ray.stepZ);
            pos.trueX += zDst * ray.ratioXtoZ;
            pos.trueY += zDst * ray.ratioYtoZ;
            if (!silent) printf("Z, X branch\n");
        } else {
            pos.trueX = (pos.x += ray.stepX);
            pos.trueY += xDst * ray.ratioYtoX;
            pos.trueZ += xDst * ray.ratioZtoX;
            if (!silent) printf("X\n");
        }
    }

    // trunc rather than floor so negative values round towards zero
    pos.x = trunc(pos.trueX);
    pos.y = trunc(pos.trueY);
    pos.z = trunc(pos.trueZ);
}

bool test(double x, double y, double z, double newX, double newY, double newZ, int interceptIterations) {

    ray = buildRay(x,y,z);

    pos.x = 0;
    pos.y = 0;
    pos.z = 0;
    pos.trueX = 0;
    pos.trueY = 0;
    pos.trueZ = 0;

    while (interceptIterations--)
        nextIntersect(true);

    bool result = pos.x == newX && pos.y == newY && pos.z == newZ;

    if (!result) {
        printf("Test %lf, %lf, %lf failed! expected %lf,%lf,%lf got %lf,%lf,%lf\n",x,y,z,newX,newY,newZ,pos.x,pos.y,pos.z);
    }
    return result;
}

bool test(double x, double y, double z, double newX, double newY, double newZ) {
    return test(x,y,z,newX,newY,newZ,1);
}


int main() {

    ray = buildRay(-0.004500,-0.022500,1);
    int iterations = 10;

    double x = 50;
    double y = 50;
    double z = 0;

    pos.x = x;
    pos.y = y;
    pos.z = z;
    pos.trueX = x;
    pos.trueY = y;
    pos.trueZ = z;

    printf("ratioYtoX: %lf\n",ray.ratioYtoX);
    printf("ratioZtoX: %lf\n",ray.ratioZtoX);
    printf("ratioYtoZ: %lf\n",ray.ratioYtoZ);
    printf("ratioXtoZ: %lf\n",ray.ratioXtoZ);
    printf("ratioXtoY: %lf\n",ray.ratioXtoY);
    printf("ratioZtoY: %lf\n",ray.ratioZtoY);

    printf("%d,%d,%d\n",ray.stepX,ray.stepY,ray.stepZ);

    while (iterations--) {
        printf("\n");
        nextIntersect(false);
        printf("pos:      %lf %lf %lf\ntrue pos: %lf %lf %lf\n",pos.x,pos.y,pos.z,pos.trueX,pos.trueY,pos.trueZ);
        if (pos.x > 45 && pos.x < 55 && pos.y > 45 && pos.y < 55 && pos.z > 5 && pos.z <= 6) {
            printf("Hit!");
            break;
        }
    }

    //return 0;

    test(1,0,0, 1,0,0);
    test(0,1,0, 0,1,0);
    test(0,0,1, 0,0,1);

    test(-1,0,0, -1,0,0);
    test(0,-1,0, 0,-1,0);
    test(0,0,-1, 0,0,-1);

    test(1,.5,0, 1,0,0);
    test(-1,.5,0, -1,0,0);
    test(-1,-.5,0, -1,0,0);
    test(1,-.5,0, 1,0,0);

    test(0,1,.5, 0,1,0);
    test(0,-1,.5, 0,-1,0);
    test(0,-1,-.5, 0,-1,0);
    test(0,1,-.5, 0,1,0);

    test(0,.5,1, 0,0,1);
    test(0,.5,-1, 0,0,-1);
    test(0,-.5,-1, 0,0,-1);
    test(0,-.5,1, 0,0,1);


    test(1,.6,0, 1,1,0,2);
    test(-1,.6,0, -1,1,0,2);
    test(-1,-.6,0, -1,-1,0,2);
    test(1,-.6,0, 1,-1,0,2);


}

