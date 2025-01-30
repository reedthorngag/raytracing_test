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

void nextIntersect();

vec4 r = vec4(1,0,0,1);
vec4 g = vec4(0,1,0,1);
vec4 b = vec4(0,0,1,1);

void main()
{

    ray = buildRay(
        cameraDir.x,// + ((gl_FragCoord.x-halfWidth)*0.001),
        cameraDir.y,// + ((gl_FragCoord.y-halfHeight)*0.001),
        cameraDir.z);

    vec3 origin1 = origin;

    //FragColor = texture(tex,vec3(gl_FragCoord.x/800,gl_FragCoord.y/600, 0.5));
    //return;

    FragColor = texture(tex, vec3(0.5,0.5,0.99));
    return;

    //FragColor = vec4(min(abs((gl_FragCoord.x-halfWidth)*0.001),1),min(abs(((gl_FragCoord.y-halfHeight)*0.001)),1),0,1);
    //return;

    // if (ray.stepX<0) {
    //     FragColor = r;
    // } else {
    //     FragColor = g;
    // }
    // return;

    //origin1.x += (gl_FragCoord.x-halfWidth);
    //origin1.y += (gl_FragCoord.y-halfHeight);

    pos.x = origin1.x;
    pos.y = origin1.y;
    pos.z = origin1.z;
    pos.trueX = origin1.x;
    pos.trueY = origin1.y;
    pos.trueZ = origin1.z;

    int scale = 100;
    bool set = false;
    for (int i = 0; i < 100; i++) {
        if (texture(tex, vec3(pos.x/scale,pos.y/scale,pos.z/scale)).xyz != vec3(0,0,0)) {
           FragColor = texture(tex, vec3(pos.x/scale,pos.y/scale,pos.z/scale));
           return;
        }
        nextIntersect();
        // nextIntersect();
        // nextIntersect();
        // if (isnan(pos.z)) {
        //     FragColor = r;
        // } else if (pos.z >= 0) {
        //     FragColor = g;
        // } else {
        //     FragColor = b;
        // }
        // set = true;
        // return;
        // vec3 a = vec3(pos.trueX,pos.trueY,pos.trueZ);
        // nextIntersect();
        // if (vec3(pos.trueX,pos.trueY,pos.trueZ) == a) {
        //     FragColor = vec4(1,0,0,1);
        //     set = true;
        //     break;
        // }
    }

    if (!set) {
        if (pos.z == 100) {
            FragColor = g;
        } else
        FragColor = vec4(0,0,0,0);
    }
    //}
    //FragColor = texture(tex, vec3())
}


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
        
        if (pos.trueX + (zDst * ray.ratioXtoZ) > pos.x + ray.stepX) {
            pos.trueX = (pos.x += ray.stepX);
            pos.trueY += xDst * ray.ratioYtoX;
            pos.trueZ += xDst * ray.ratioZtoX;
        } else {
            pos.trueZ = (pos.z += ray.stepZ);
            pos.trueX += zDst * ray.ratioXtoZ;
            pos.trueY += zDst * ray.ratioYtoZ;
        }
    }

    pos.x = floor(pos.trueX + 0.00000001);
    pos.y = floor(pos.trueY + 0.00000001);
    pos.z = pos.trueZ;//floor(pos.trueZ + 0.00000001);
}