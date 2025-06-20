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

            int y = round(o1.eval(x*0.005,z*0.005)*30) + round(o2.eval(x*0.05,z*0.05) * 5) + round(o3.eval(x*0.1,z*0.1) * 3) + 32;

            if (y < 20) {
                for (int i = 20; i > y; i--) {
                    putBlock(Pos{x,i,z},Block{REFRACTIVE | LIQUID, RGB_TO_U64(0,150,10), 0}, 6);
                }
                putBlock(Pos{x,y,z},Block{NONE,RGB_TO_U64(45,18,0),0},6);
            } else
                putBlock(Pos{x,y,z},Block{NONE,RGB_TO_U64(0,150,10),0},6);
            y--;

            for (int i = 3; y > 0 && i; i--, y--) {
                putBlock(Pos{x,y,z},Block{NONE,RGB_TO_U64(45,18,0),0},6);
            }

            for (; y > 0; y--) {
                putBlock(Pos{x,y,z},Block{NONE,RGB_TO_U64(33,33,33),0},6);
            }
        }
    }
}
