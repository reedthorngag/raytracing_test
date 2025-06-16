
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

    RayResult castRayFromCam(int steps) {

        Ray ray = buildRay(cameraDir);

        RayPos pos;
        pos.round = ivec3(trunc(cameraPos));
        pos.exact = cameraPos;

        if (ray.step.x < 0) pos.exact.x -= 1;
        if (ray.step.y < 0) pos.exact.y -= 1;
        if (ray.step.z < 0) pos.exact.z -= 1;

        pos.deltaPos = ray.absDelta - (pos.exact - glm::dvec3(pos.round)) * ray.delta;

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
            Block block = getBlock(Pos{pos.round.x,pos.round.y,pos.round.z});
            if (block.color != -1ull && (block.flags & 0x10) == 0)
                return RayResult{pos.round,lastPos,steps};
        }

        return RayResult{pos.round,lastPos,0};
    }

}
