#include <glm/glm.hpp>

#include "types.hpp"
#include "globals.hpp"

struct RayResult {
    glm::ivec3 pos;
    glm::ivec3 lastPos;
    int steps;
};

namespace RAY_CASTER {

    RayResult castRayFromCam(int steps);

}


