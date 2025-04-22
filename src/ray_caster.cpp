
#include "ray_caster.hpp"
#include "voxel_data/tetrahexa_tree.hpp"

using namespace glm;

namespace RAY_CASTER {

    struct Ray {
        vec3 dir;

        ivec3 step;

        dvec3 delta;

        dvec3 absDelta;
    };

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

    vec3 castRayFromCam(int steps) {

        Ray ray = buildRay(cameraDir);

        RayPos pos;
        pos.round = ivec3(floor(cameraPos));
        pos.exact = cameraPos;

        if (ray.step.x < 0) pos.exact.x -= 1;
        if (ray.step.y < 0) pos.exact.y -= 1;
        if (ray.step.z < 0) pos.exact.z -= 1;

        pos.deltaPos.x = ray.absDelta.x - (pos.exact.x - pos.round.x) * ray.delta.x;
        pos.deltaPos.y = ray.absDelta.y - (pos.exact.y - pos.round.y) * ray.delta.y;
        pos.deltaPos.z = ray.absDelta.z - (pos.exact.z - pos.round.z) * ray.delta.z;

        ivec3 lastPos;
        while (steps--) {
            lastPos = pos.round;
            if (pos.deltaPos.x < pos.deltaPos.y && pos.deltaPos.x < pos.deltaPos.z) {
                pos.round.x += ray.step.x;
                pos.deltaPos.x += ray.absDelta.x;
            } else if (pos.deltaPos.y < pos.deltaPos.z) {
                pos.round.y += ray.step.y;
                pos.deltaPos.y += ray.absDelta.y;
            } else {
                pos.round.z += ray.step.z;
                pos.deltaPos.z += ray.absDelta.z;
            }
            if (getBlock(Pos{pos.round.x,pos.round.y,pos.round.z}) != -1ull)
                return lastPos;
        }

        return pos.round;
    }

}
