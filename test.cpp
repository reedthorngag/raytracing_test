#include <glm/glm.hpp>
#include <stdio.h>

using namespace glm;

struct Ray {
    vec3 dir;

    ivec3 step;

    dvec3 ratiosX;
    dvec3 ratiosY;
    dvec3 ratiosZ;

    dvec3 delta;

    dvec3 absDelta;
};

double ratio(double a, double b) {
    if (a == 0 || b == 0) {
        return 0;
    }
    return a / b;
}

Ray buildRay(vec3 dir) {
    Ray ray;

    ray.dir = dir;

    ray.step.x = 1;
    ray.step.y = 1;
    ray.step.z = 1;

    if (dir.x < 0)
        ray.step.x = -1;
    if (dir.y < 0)
        ray.step.y = -1;
    if (dir.z < 0)
        ray.step.z = -1;
    
    ray.ratiosX = dvec3(
        1,
        ratio(dir.y,dir.x),
        ratio(dir.z,dir.x)
    );
    ray.ratiosY = dvec3(
        ratio(dir.x,dir.y),
        1,
        ratio(dir.z,dir.y)
    );
    ray.ratiosZ = dvec3(
        ratio(dir.x,dir.z),
        ratio(dir.y,dir.z),
        1
    );

    ray.delta.x = 1/dir.x;
    ray.delta.y = 1/dir.y;
    ray.delta.z = 1/dir.z;

    ray.absDelta.x = abs(ray.delta.x);
    ray.absDelta.y = abs(ray.delta.y);
    ray.absDelta.z = abs(ray.delta.z);

    return ray;
}

struct RayPos {
    ivec3 round;
    
    dvec3 exact;

    dvec3 deltaPos;
};

int main() {

    glm::vec3 camDir = normalize(vec3(11,20,3));
    printf("cam dir: %f, %f, %f\n",camDir.x,camDir.y,camDir.z);
    glm::vec3 camPos = vec3(10.5, 12.1, 14.7);

    Ray ray = buildRay(camDir);

    RayPos pos;
    pos.round = ivec3(trunc(camPos));
    pos.exact = camPos;

    if (ray.step.x < 0) pos.exact.x -= 1;
    if (ray.step.y < 0) pos.exact.y -= 1;
    if (ray.step.z < 0) pos.exact.z -= 1;

    pos.deltaPos = ray.absDelta - (pos.exact - glm::dvec3(pos.round)) * ray.delta;

    int lastHit = 0;
    int steps = 500;
    while (steps--) {
        if (pos.deltaPos.x < pos.deltaPos.y && pos.deltaPos.x < pos.deltaPos.z) {
            pos.round.x += ray.step.x;
            pos.deltaPos.x += ray.absDelta.x;
            lastHit = 0;

        } else if (pos.deltaPos.y < pos.deltaPos.z) {
            pos.round.y += ray.step.y;
            pos.deltaPos.y += ray.absDelta.y;
            lastHit = 1;

        } else {
            pos.round.z += ray.step.z;
            pos.deltaPos.z += ray.absDelta.z;
            lastHit = 2;
        }
    }

    printf("lastHit: %d\n",lastHit);
    printf("pos.round: %d, %d, %d\n",pos.round.x,pos.round.y,pos.round.z);

    double val = pos.round[lastHit] - floor(camPos[lastHit]);
    if (lastHit == 0) {
        printf("ray.ratiosX: %f, %f, %f\n",ray.ratiosX.x,ray.ratiosX.y,ray.ratiosX.z);
        pos.exact = vec3(val * ray.ratiosX);
    } else if (lastHit == 1) {
        printf("ray.ratiosY: %f, %f, %f\n",ray.ratiosY.x,ray.ratiosY.y,ray.ratiosY.z);
        pos.exact = vec3(val * ray.ratiosY);
    } else if (lastHit == 2) {
        printf("ray.ratiosZ: %f, %f, %f\n",ray.ratiosZ.x,ray.ratiosZ.y,ray.ratiosZ.z);
        pos.exact = vec3(val * ray.ratiosZ);
    }
    pos.exact += camPos;

    printf("pos.exact: %f, %f, %f\n",pos.exact.x,pos.exact.y,pos.exact.z);
    return 0;
}
