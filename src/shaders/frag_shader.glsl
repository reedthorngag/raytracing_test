#version 460 core

out vec4 FragColor;

uniform vec3 origin;
uniform vec3 cameraDir;

uniform sampler3D tex;

in vec4 gl_FragCoord;

int width = 800;
int halfWidth = width/2;
int height = 600;
int halfHeight = height/2;

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
    float x;
    float y;
    float z;
    float trueX;
    float trueY;
    float trueZ;
};

Pos pos;
Ray ray;

void nextIntersect();

void main()
{

    ray = buildRay(
        cameraDir.x,// + ((gl_FragCoord.x-halfWidth)*0.0001),
        cameraDir.y,// + ((gl_FragCoord.y-halfHeight)*0.0001),
        cameraDir.z);

    vec3 origin1 = origin;

    //FragColor = vec4(min(abs((gl_FragCoord.x-halfWidth)*0.001),1),min(abs(((gl_FragCoord.y-halfHeight)*0.001)),1),0,1);
    //return;

    origin1.x += (gl_FragCoord.x-halfWidth);
    origin1.y += (gl_FragCoord.y-halfHeight);

    pos.x = origin1.x;
    pos.y = origin1.y;
    pos.z = origin1.z;
    pos.trueX = origin1.x;
    pos.trueY = origin1.y;
    pos.trueX = origin1.z;

    int scale = 100;
    bool set = false;
    for (int i = 0; i < 100; i++) {
        if (texture(tex, vec3(pos.x/scale,pos.y/scale,pos.z/scale)).xyz != vec3(0,0,0)) {
           FragColor = texture(tex, vec3(pos.x/scale,pos.y/scale,pos.z/scale));
           set = true;
           break;
        }
        nextIntersect();
        // vec3 a = vec3(pos.trueX,pos.trueY,pos.trueZ);
        // nextIntersect();
        // if (vec3(pos.trueX,pos.trueY,pos.trueZ) == a) {
        //     FragColor = vec4(1,0,0,1);
        //     set = true;
        //     break;
        // }
    }

    if (!set) {
        FragColor = vec4(0,0,0,0);
    }
    //}
    //FragColor = texture(tex, vec3())
}


void nextIntersect() {

    int step = 1;

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

    pos.x = floor(pos.trueX + 0.00000001);
    pos.y = floor(pos.trueY + 0.00000001);
    pos.z = floor(pos.trueZ + 0.00000001);
}