#version 460 core
#extension GL_ARB_gpu_shader_int64 : enable

#define u64 uint64_t

out vec4 FragColor;

uniform vec3 origin;
uniform vec3 cameraDir;
uniform vec2 mousePos;
uniform int renderPosData;

uniform sampler3D tex;

in vec4 gl_FragCoord;

int width = 1920;
int halfWidth = width/2;
int height = 991;
int halfHeight = height/2;

float aspect_ratio = float(height) / width;

double pixelWidth = 1.0/width;
double pixelHeight = 1.0/height;

double scale = 1.0/100;

float projection_plane_width = 1 * tan(radians(45));
float projection_plane_height = projection_plane_width * aspect_ratio;

struct Ray {
    vec3 dir;

    ivec3 step;

    ivec3 sign;

    double ratioYtoX;
    double ratioYtoZ;
    double ratioXtoY;
    double ratioXtoZ;
    double ratioZtoX;
    double ratioZtoY;

    dvec3 delta;

    dvec3 absDelta;
};

double ifZeroMakeOne(double n) {
    return n == 0 ? 1 : n;
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

Ray buildRay(vec3 dir) {
    Ray ray;

    ray.dir = dir;

    ray.step.x = 1;
    ray.step.y = 1;
    ray.step.z = 1;

    //ray.sign = dvec3(0);

    if (dir.x < 0)
        ray.step.x = -1;
        //ray.sign.x = 1 << 31;
    if (dir.y < 0)
        ray.step.y = -1;
        //ray.sign.y = 1 << 31;
    if (dir.z < 0)
        ray.step.z = -1;
        //ray.sign.z = 1 << 31;

    ray.ratioYtoX = matchSign(makeRatio(dir.y,dir.x),ray.step.y);
    ray.ratioYtoZ = matchSign(makeRatio(dir.y,dir.z),ray.step.y);
    ray.ratioXtoY = matchSign(makeRatio(dir.x,dir.y),ray.step.x);
    ray.ratioXtoZ = matchSign(makeRatio(dir.x,dir.z),ray.step.x);
    ray.ratioZtoX = matchSign(makeRatio(dir.z,dir.x),ray.step.z);
    ray.ratioZtoY = matchSign(makeRatio(dir.z,dir.y),ray.step.z);

    ray.delta.x = 1/dir.x;
    ray.delta.y = 1/dir.y;
    ray.delta.z = 1/dir.z;

    ray.absDelta.x = abs(ray.delta.x);
    ray.absDelta.y = abs(ray.delta.y);
    ray.absDelta.z = abs(ray.delta.z);

    return ray;
}

struct Pos {
    ivec3 round;
    
    dvec3 exact;

    dvec3 deltaPos;
};

Pos pos;
Ray ray;

void nextIntersect(int step);
void nextIntersectDDA();

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
    FragColor = vec4(vec.xyz,0);
}

void main()
{

    if (renderPosData == 0 && distance(gl_FragCoord.xy, mousePos) < 3) {
        FragColor = vec4(1,1,1,1);
        return;
    }

    if (texture(tex, vec3(origin.x*scale,origin.y*scale,origin.z*scale)).xyz != 0) {
        FragColor = texture(tex, vec3(origin.x*scale,origin.y*scale,origin.z*scale));
        return;
    }

    vec3 camLeft = cross(cameraDir,vec3(0,1,0));
    vec3 camUp = cross(cameraDir,camLeft);

    float FragCoordX = float(gl_FragCoord.x * pixelWidth) - 0.5;
    float FragCoordY = float(gl_FragCoord.y * pixelHeight) - 0.5;
    
    vec3 projection_plane_center = cameraDir;
    vec3 projection_plane_left = normalize(cross(projection_plane_center, vec3(0,1,0)));
    vec3 projection_plane_intersect = normalize(projection_plane_center + 
        (projection_plane_left * -(projection_plane_width * FragCoordX)) + 
        cross(projection_plane_center, projection_plane_left) * -FragCoordY * projection_plane_height);
    

    ray = buildRay(projection_plane_intersect);

    pos.round = ivec3(floor(origin));

    pos.exact = origin;
    
    if (ray.step.x < 0) pos.exact.x -= 1;
    if (ray.step.y < 0) pos.exact.y -= 1;
    if (ray.step.z < 0) pos.exact.z -= 1;

    int step = 1;

    pos.deltaPos.x = ray.absDelta.x - (pos.exact.x - pos.round.x) * ray.delta.x;
    pos.deltaPos.y = ray.absDelta.y - (pos.exact.y - pos.round.y) * ray.delta.y;
    pos.deltaPos.z = ray.absDelta.z - (pos.exact.z - pos.round.z) * ray.delta.z;

    bool set = false;
    for (int i = 0; i < 100; i++) {
        if (texture(tex, vec3(pos.round.x*scale,pos.round.y*scale,pos.round.z*scale)).xyz != 0) {
           FragColor = texture(tex, vec3(pos.round.x*scale,pos.round.y*scale,pos.round.z*scale));
           set = true;
           break;
        }
        nextIntersectDDA();
        //nextIntersect(step);
    }

    nextIntersectDDA();

    if (!set) {
        FragColor = vec4(0,0,( abs(double(pos.round.y) - 50))/100,0);
    }
    if (renderPosData == 1)
        FragColor = vec4(pos.round.x,pos.round.y,pos.round.z,0);
    else if (renderPosData == 2)
        FragColor = vec4(ray.dir.x,ray.dir.y,ray.dir.z,0);
    else if (renderPosData == 3)
        setFragToVec(projection_plane_center);
    else if (renderPosData == 4)
        FragColor = vec4(projection_plane_left,projection_plane_width);
    else if (renderPosData == 5)
        FragColor = vec4(ray.delta.x,ray.delta.y,ray.delta.z,0);
    else if (renderPosData == 6)
        FragColor = vec4(origin.xyz,0);
    else if (renderPosData == 7)
        setFragToVec(cameraDir);
    else if (renderPosData == 8)
        setFragToVec(trunc(origin.xyz));
    else if (renderPosData == 9)
        setFragToVec(vec3(0));
    else if (renderPosData == 10)
        setFragToVec(vec3(0));
        
}

void nextIntersectDDA() {

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
}

// need to work out how to update pos.deltaPos more efficiently if possible
void nextIntersect(int step) {

    ivec3 steps = ray.step * step;

    double xDst = (pos.round.x + steps.x) - pos.exact.x;
    double yDst = (pos.round.y + steps.y) - pos.exact.y;
    double zDst = (pos.round.z + steps.z) - pos.exact.z;

    if (pos.deltaPos.x < pos.deltaPos.y && pos.deltaPos.x < pos.deltaPos.z) {

        pos.exact.x += xDst;
        pos.round.x += steps.x;
        xDst = abs(xDst);
        pos.exact.y += xDst * ray.ratioYtoX;
        pos.exact.z += xDst * ray.ratioZtoX;

    } else if (pos.deltaPos.y < pos.deltaPos.z) {
        
        pos.exact.y += yDst;
        pos.round.y += steps.y;
        yDst = abs(yDst);
        pos.exact.x += yDst * ray.ratioXtoY;
        pos.exact.z += yDst * ray.ratioZtoY;

    } else {
        pos.exact.z += zDst;
        pos.round.z += steps.z;
        zDst = abs(zDst);
        pos.exact.x += zDst * ray.ratioXtoZ;
        pos.exact.y += zDst * ray.ratioYtoZ;
    }

    pos.deltaPos.x = ray.absDelta.x - (pos.exact.x - pos.round.x) * ray.delta.x;
    pos.deltaPos.y = ray.absDelta.y - (pos.exact.y - pos.round.y) * ray.delta.y;
    pos.deltaPos.z = ray.absDelta.z - (pos.exact.z - pos.round.z) * ray.delta.z;
}
