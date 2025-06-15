#include <OpenSimplexNoise.h>

#include "voxel_data/types.hpp"
#include "voxel_data/tetrahexa_tree.hpp"
#include "types.hpp"
#include "world_gen.hpp"

const int WIDTH = 200;
const int LENGTH = 200;

using namespace OpenSimplexNoise;

void genWorld() {

    Noise o1(42);
    Noise o2(64);
    Noise o3(100);

    for (int x = 0; x < WIDTH; x++) {
        for (int z = 0; z < LENGTH; z++) {

            int y = round(o1.eval(x*0.01,z*0.01)*20) + round(o2.eval(x*0.1,z*0.1)*5) + 20;

            putBlock(Pos{x,y,z},Block{NONE,RGB_TO_U64(0,150,10),0},6);
            y--;

            for (int i = 3; y && i; i--, y--) {
                putBlock(Pos{x,y,z},Block{NONE,RGB_TO_U64(45,18,0),0},6);
            }

            for (; y; y--) {
                putBlock(Pos{x,y,z},Block{NONE,RGB_TO_U64(33,33,33),0},6);
            }
        }
    }
}
