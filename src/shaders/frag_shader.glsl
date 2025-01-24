#version 330 core
out vec4 FragColor;

uniform vec3 pos;
uniform vec3 cameraDir;

uniform sampler3D tex;

in vec4 gl_FragCoord;


struct Ray {
    float x;
    float y;
    float z;

    float ratioYtoX;
    float ratioYtoZ;
    float ratioXtoY;
    float ratioXtoZ;
    float ratioZtoX;
    float ratioZtoY;
};

Ray buildRay(float x, float y, float z) {
    Ray ray;

    ray.x = x;
    ray.y = y;
    ray.z = z;

    ray.ratioYtoX = y / x;
    ray.ratioYtoZ = y / z;
    ray.ratioXtoY = x / y;
    ray.ratioXtoZ = x / z;
    ray.ratioZtoX = z / x;
    ray.ratioZtoY = z / y;

    return ray;
}

struct Pos {
    int x;
    int y;
    int z;
    float trueX;
    float trueY;
    float trueZ;
};

void nextIntersect(Pos pos, Ray ray, int step) {

    float xDst = (pos.x + step) - pos.trueX;
    float yDst = (pos.y + step) - pos.trueY;
    float zDst = (pos.z + step) - pos.trueZ;

    if (pos.trueY + (xDst * ray.ratioYtoX) > pos.y + step) {
        // next intercept is with x,y+1

        if (pos.trueZ + (yDst * ray.ratioZtoY) > pos.z + step) {
            pos.trueZ = (pos.z += step);
            pos.trueX += zDst * ray.ratioXtoZ;
            pos.trueY += zDst * ray.ratioYtoZ;
        } else {
            pos.trueY = (pos.y += step);
            pos.trueX += yDst * ray.ratioXtoY;
            pos.trueZ += yDst * ray.ratioZtoY;
        }
    } else {
        // next intercept is with x+1,y
        
        if (pos.trueZ + (xDst * ray.ratioZtoX) > pos.z + step) {
            pos.trueZ = (pos.z += step);
            pos.trueX += zDst * ray.ratioXtoZ;
            pos.trueY += zDst * ray.ratioYtoZ;
        } else {
            pos.trueX = (pos.x += step);
            pos.trueY += xDst * ray.ratioYtoX;
            pos.trueZ += xDst * ray.ratioZtoX;
        }
    }

    pos.x = int(floor(pos.trueX + 0.00000001));
    pos.y = int(floor(pos.trueY + 0.00000001));
    pos.z = int(floor(pos.trueZ + 0.00000001));
}

void main()
{

    Ray ray = buildRay(
        cameraDir.x + ((gl_FragCoord.x-0.5)*2),
        cameraDir.y + ((gl_FragCoord.y-0.5)),
        cameraDir.z);
    Pos pos;
    pos.x = int(pos.x);
    pos.y = int(pos.y);
    pos.z = int(pos.z);
    pos.trueX = pos.x;
    pos.trueY = pos.y;
    pos.trueX = pos.z;

    int scale = 100000;
    bool set = false;
    for (int i = 0; i < 200; i++) {
        if (texture(tex, vec3(pos.x/scale,pos.y/scale,pos.z/scale)).xyz != vec3(0,0,0)) {
            FragColor = texture(tex, vec3(pos.x/scale,pos.y/scale,pos.z/scale));
            set = true;
        //    break;
        }
        nextIntersect(pos,ray,1);
    }

    if (!set) {
        FragColor = vec4(0,0,0,0);
    }

    //FragColor = texture(tex, vec3())
} 