#version 460 core

out vec4 FragColor;

uniform vec3 origin;
uniform vec3 cameraDir;
uniform vec2 mousePos;
uniform int renderPosData;

uniform sampler3D tex;
uniform sampler2D pixelTex;

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

    double deltaX;
    double deltaY;
    double deltaZ;
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

double makeRatio(double a, double b) {
    if ((a < 0 && b > 0) || (a > 0 && b < 0)) {
        //a += -b;b = 0;//return abs(a + -b);
    }
    return abs(a) / abs(ifZeroMakeOne(b));
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

    ray.ratioYtoX = matchSign(makeRatio(y,x),ray.stepY);
    ray.ratioYtoZ = matchSign(makeRatio(y,z),ray.stepY);
    ray.ratioXtoY = matchSign(makeRatio(x,y),ray.stepX);
    ray.ratioXtoZ = matchSign(makeRatio(x,z),ray.stepX);
    ray.ratioZtoX = matchSign(makeRatio(z,x),ray.stepZ);
    ray.ratioZtoY = matchSign(makeRatio(z,y),ray.stepZ);

    ray.deltaX = 1/x;
    ray.deltaY = 1/y;
    ray.deltaZ = 1/z;

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

bool nextIntersect();
bool nextIntersect2();

vec4 r = vec4(1,0,0,1);
vec4 g = vec4(0,1,0,1);
vec4 b = vec4(0,0,1,1);

mat4 rotationMatrix(vec3 axis, float angle)
{
    axis = normalize(axis);
    float s = sin(angle);
    float c = cos(angle);
    float oc = 1.0 - c;
    
    return mat4(oc * axis.x * axis.x + c,           oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,  0.0,
                oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,  0.0,
                oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c,           0.0,
                0.0,                                0.0,                                0.0,                                1.0);
}

void setFragToVec(vec3 vec) {
    FragColor = vec4(vec.x,vec.y,vec.z,0);
}

void main()
{

    if (renderPosData == 0 && distance(gl_FragCoord.xy, mousePos) < 3) {
        FragColor = vec4(1,1,1,1);
        return;
    }

    vec3 camLeft = cross(cameraDir,vec3(0,1,0));
    vec3 camUp = cross(cameraDir,camLeft);

    float FragCoordX = (gl_FragCoord.x / width) - 0.5;
    float FragCoordY = (gl_FragCoord.y / height) - 0.5;

    float projection_plane_width = 1 * tan(radians(45));
    
    vec3 projection_plane_center = cameraDir;
    vec3 projection_plane_left = normalize(cross(projection_plane_center, vec3(0,1,0)));
    vec3 projection_plane_intersect = projection_plane_center + (projection_plane_left * -(projection_plane_width * FragCoordX)) + cross(projection_plane_center,projection_plane_left) * -FragCoordY;
    
    vec3 rayDir = normalize(projection_plane_intersect);

    ray = buildRay(
        rayDir.x,
        rayDir.y,
        rayDir.z);

    vec3 origin1 = origin;

    //FragColor = vec4(min(abs((gl_FragCoord.x-halfWidth)*0.001),1),min(abs(((gl_FragCoord.y-halfHeight)*0.001)),1),0,1);
    //return;

    //origin1.x += (gl_FragCoord.x-halfWidth);
    //origin1.y += (gl_FragCoord.y-halfHeight);

    pos.x = trunc(origin1.x) + matchSign(0.5,origin1.x);
    pos.y = trunc(origin1.y) + matchSign(0.5,origin1.y);
    pos.z = trunc(origin1.z) + matchSign(0.5,origin1.z);
    pos.trueX = origin1.x;
    pos.trueY = origin1.y;
    pos.trueZ = origin1.z;

    int scale = 100;

    if (texture(tex, vec3(origin.x/scale,origin.y/scale,origin.z/scale)).xyz != vec3(0,0,0)) {
        FragColor = texture(tex, vec3(origin.x/scale,origin.y/scale,origin.z/scale));
        return;
    }

    bool set = false;
    for (int i = 0; i < 100; i++) {
        if (texture(tex, vec3(pos.x/scale,pos.y/scale,pos.z/scale)).xyz != vec3(0,0,0)) {
           FragColor = texture(tex, vec3(pos.x/scale,pos.y/scale,pos.z/scale));
           set = true;
           break;
        }
        if (nextIntersect()) return;
    }

    if (!set) {
        if (pos.x > 55 || pos.x < 45 || pos.y > 55 || pos.y < 45) {
            FragColor = vec4(0,0,( abs(pos.y - 50))/100,0);
        } else
            FragColor = vec4(0,0,0,0);
    }
    if (renderPosData == 1)
        FragColor = vec4(pos.x,pos.y,pos.z,0);
    else if (renderPosData == 2)
        FragColor = vec4(ray.x,ray.y,ray.z,0);
    else if (renderPosData == 3)
        setFragToVec(projection_plane_center);
    else if (renderPosData == 4)
        FragColor = vec4(projection_plane_left,projection_plane_width);
    else if (renderPosData == 5)
        FragColor = vec4(ray.deltaX,ray.deltaY,ray.deltaZ,0);
    else if (renderPosData == 6)
        FragColor = vec4(origin1.xyz,0);
    else if (renderPosData == 7)
        setFragToVec(cameraDir);
    else if (renderPosData == 8)
        setFragToVec(projection_plane_intersect);
    else if (renderPosData == 9)
        setFragToVec(vec3(0));
    else if (renderPosData == 10)
        setFragToVec(origin);
        
    
}

bool nextIntersect() {

    double xDst = abs((pos.x + ray.stepX) - pos.trueX);
    double yDst = abs((pos.y + ray.stepY) - pos.trueY);
    double zDst = abs((pos.z + ray.stepZ) - pos.trueZ);

    double xAbsDst = abs(xDst * ray.deltaX);
    double yAbsDst = abs(yDst * ray.deltaY);
    double zAbsDst = abs(zDst * ray.deltaZ);

    if (xAbsDst < yAbsDst && xAbsDst < zAbsDst) {

        pos.trueX += (pos.x + ray.stepX) - pos.trueX;
        pos.x += ray.stepX;
        pos.trueY += xDst * ray.ratioYtoX;
        pos.trueZ += xDst * ray.ratioZtoX;

    } else if (yAbsDst < zAbsDst) {
        
        pos.trueY += (pos.y + ray.stepY) - pos.trueY;
        pos.y += ray.stepY;
        pos.trueX += yDst * ray.ratioXtoY;
        pos.trueZ += yDst * ray.ratioZtoY;

    } else {
        pos.trueZ += (pos.z + ray.stepZ) - pos.trueZ;
        pos.z += ray.stepZ;
        pos.trueX += zDst * ray.ratioXtoZ;
        pos.trueY += zDst * ray.ratioYtoZ;
    }

    return false;
}
