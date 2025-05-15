#version 460 core
#extension GL_ARB_gpu_shader_int64 : enable
#extension GL_NV_gpu_shader5 : enable

#define u64 uint64_t
#define i64 int64_t
#define u32 uint
#define u8 uint8_t

out vec4 FragColor;
//out layout (location = 0) vec4 FragOut;
//out layout (location = 1) vec4 FragOut2;

uniform vec3 origin;
uniform vec3 cameraDir;
uniform vec2 mousePos;
uniform int renderPosData;
uniform uint rootNodeIndex;
uniform uint originMortonPos;
uniform vec2 pixelSize;
uniform vec2 projPlane;
uniform uvec2 size;

in vec4 gl_FragCoord;

layout (std430, binding = 3) readonly buffer layoutNodes {
    u64vec2 nodes[];
};

layout (packed, binding = 2) readonly buffer layoutArrays {
    uint[64] children_array[];
};


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
    
    vec3 exact;

    dvec3 deltaPos;
};

Pos pos;
Ray ray;

const int MAX_DEPTH = 6;

u32 stack[MAX_DEPTH];
uint currentMortonPos;
int posOffset;
int depth;
uint mortonPos = 0;

u64 getBlock();
void nextIntersect(int step);
void nextIntersectDDA();

mat4 rotationMatrix(vec3 axis, float angle) {
    axis = normalize(axis);
    float s = sin(angle);
    float c = cos(angle);
    float oc = 1.0 - c;
    
    return mat4(oc * axis.x * axis.x + c,           oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,  0.0,
                oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,  0.0,
                oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c,           0.0,
                0.0,                                0.0,                                0.0,                                1.0);
}

const int BITS_PER_COLOR = 21;
const int COLOR_RANGE = 1 << BITS_PER_COLOR;
const int COLOR_MASK = COLOR_RANGE - 1;
const double SCALED_COLOR = 1.0 / COLOR_RANGE;

vec4 color_int_to_vec4(u64 color) {
    return vec4((color >> (BITS_PER_COLOR * 2)) * SCALED_COLOR,
        ((color >> BITS_PER_COLOR) & COLOR_MASK) * SCALED_COLOR,
        (color & COLOR_MASK) * SCALED_COLOR,
        1.0
    );
}

float sigmoid(float x, float scale, float dropOffSteepness) {
    return abs(1.0/(1+exp(-x*dropOffSteepness))*scale);
}

vec4 genSkyBox() {
    if (ray.dir.y < 0) ray.dir.y *= 1.4;
    float haze = (0.1-abs(clamp(ray.dir.y,-.3,.3))) * 0.8 + 0.1;
    float modifier = sigmoid(1-(haze*2),1.0,2.0);
    vec3 hazeMask = vec3(haze);
    vec3 skyDefaultColor = vec3(0.2,0.4,1);
    return vec4((skyDefaultColor + clamp(haze,0,1) * 3)*modifier,1);
}

vec4 r = vec4(1,0,0,1);
vec4 g = vec4(0,1,0,1);
vec4 b = vec4(0,0,1,1);

vec3 lastHit = vec3(0);

vec3[] ratioMap = {
    vec3(1,ray.ratioXtoY,ray.ratioXtoZ),
    vec3(ray.ratioYtoX,1,ray.ratioYtoZ),
    vec3(ray.ratioZtoX,ray.ratioZtoY,1)
};

u64 castRay(vec3 dir, vec3 startPos, uint maxSteps) {
    buildRay(dir);
    pos.exact = startPos;
    pos.round = ivec3(floor(startPos));

    if (ray.step.x < 0) pos.exact.x -= 1;
    if (ray.step.y < 0) pos.exact.y -= 1;
    if (ray.step.z < 0) pos.exact.z -= 1;

    pos.deltaPos.x = ray.absDelta.x - (pos.exact.x - pos.round.x) * ray.delta.x;
    pos.deltaPos.y = ray.absDelta.y - (pos.exact.y - pos.round.y) * ray.delta.y;
    pos.deltaPos.z = ray.absDelta.z - (pos.exact.z - pos.round.z) * ray.delta.z;

    for (int i = 0; i < 100; i++) {

        nextIntersectDDA();

        u64 color = getBlock();
        if (color != -1) {
            return color;
        }
    }

    return -1;
}

// generate transform matrix code adapted from https://columbusutrigas.com/posts/rtgi/
mat3 computeTransformMat(vec3 normal)
{
    const vec3 up = vec3(0,1,0);
    const vec3 xAxis = normalize(cross(up, normal));
    const vec3 yAxis = cross(normal, xAxis);
    const vec3 zAxis = normal;

    return mat3(xAxis, yAxis, zAxis);
}

// fibonacci sphere algorithm from stack overflow https://stackoverflow.com/a/26127012/15233212
// ported to glsl and modified by me
float phi = radians(180) * (sqrt(5.0) - 1.0); // golden angle in radians

u64 raycastHemisphere(u64 color) {

    vec3 lastHitVal = pos.round * lastHit;
    vec3 pos = max() * ratioMap[];
    vec3 normal = -ray.step * lastHit;
    mat3 transformMat = computeTransformMat(normal);

    int numRays = 20;
    for (int i = 0; i < numRays; i++) {
        y = 1 - float(i) / (numRays - 1) * 2; // y goes from 1 to -1
        radius = sqrt(1-y*y);

        theta = phi * i;

        x = cos(theta) * radius;
        z = sin(theta) * radius;

        u64 color2 = castRay(transformMat * vec3(x,y,z), pos, 10);
        if (color2 == color) color = g;
    }

    return color;
}

void main()
{

    float pixelWidth = 1.0/size.x;
    float pixelHeight = 1.0/size.y;

    stack[0] = 0;
    depth = 0;

    pos.round = ivec3(floor(origin));

    mortonPos = originMortonPos;
    currentMortonPos = mortonPos;

    u64 color = getBlock();

    if (color != -1) {
        FragColor = color_int_to_vec4(color);
        return;
    }

    vec2 FragCoord = gl_FragCoord.xy * vec2(pixelWidth,pixelHeight);//pixelSize;

    vec3 camLeft = cross(cameraDir,vec3(0,1,0));
    vec3 camUp = cross(cameraDir,camLeft);
    vec3 projection_plane_center = cameraDir;
    vec3 projection_plane_left = normalize(cross(projection_plane_center, vec3(0,1,0)));
    vec3 projection_plane_intersect = normalize(projection_plane_center + 
        (projection_plane_left * -(projPlane.x * (FragCoord.x - 0.5))) + 
        cross(projection_plane_center, projection_plane_left) * (-FragCoord.y + 0.5) * projPlane.y);
    

    ray = buildRay(projection_plane_intersect);

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

        nextIntersectDDA();

        color = getBlock();
        if (color != -1) {
            color = raycastHemisphere(color);
            FragColor = color_int_to_vec4(color);
            return;
        }
    }

    FragColor = genSkyBox();

    if (renderPosData == 1) {
        FragColor = vec4(pixelWidth*100,pixelHeight*100,1,0);
    }


    return;

    // if (pos.deltaPos.x < pos.deltaPos.y && pos.deltaPos.x < pos.deltaPos.z) {
    //     pos.round.x += ray.step.x;
    //     pos.exact = pos.round.x * vec3(1,ray.ratioXtoY,ray.ratioXtoZ);
    // } else if (pos.deltaPos.y < pos.deltaPos.z) {
    //     pos.round.y += ray.step.y;
    //     pos.exact = pos.round.y * vec3(ray.ratioYtoX,1,ray.ratioYtoZ);
    // } else {
    //     pos.round.z += ray.step.z;
    //     pos.exact = pos.round.z * vec3(ray.ratioZtoX,ray.ratioZtoY,1);
    // }
    // pos.exact -= ray.dir;
    // FragOut = vec4(abs(pos.exact-origin), distance(pos.exact,origin));
    // FragOut2 = vec4(ray.dir,0);
}

void nextIntersectDDA() {

    if (pos.deltaPos.x < pos.deltaPos.y && pos.deltaPos.x < pos.deltaPos.z) {
        pos.round.x += ray.step.x;
        pos.deltaPos.x += ray.absDelta.x;

        uint n = pos.round.x;
        n = (n | (n << 16)) & 0x030000FF;
        n = (n | (n <<  8)) & 0x0300F00F;
        n = (n | (n <<  4)) & 0x030C30C3;
        mortonPos &= 0xFCF3CF3C;
        mortonPos |= n;

        lastHit = vec3(1,0,0);

    } else if (pos.deltaPos.y < pos.deltaPos.z) {
        pos.round.y += ray.step.y;
        pos.deltaPos.y += ray.absDelta.y;

        uint n = pos.round.y;
        n = (n | (n << 16)) & 0x030000FF;
        n = (n | (n <<  8)) & 0x0300F00F;
        n = (n | (n <<  4)) & 0x030C30C3;
        mortonPos &= 0xFCF3CF3C << 2 | 0x3;
        mortonPos |= n << 2;

        lastHit = vec3(0,1,0);

    } else {
        pos.round.z += ray.step.z;
        pos.deltaPos.z += ray.absDelta.z;
        
        uint n = pos.round.z;
        n = (n | (n << 16)) & 0x030000FF;
        n = (n | (n <<  8)) & 0x0300F00F;
        n = (n | (n <<  4)) & 0x030C30C3;
        mortonPos &= 0xFCF3CF3C << 4 | 0xf;
        mortonPos |= n << 4;

        lastHit = vec3(0,0,1);
    }
}

u64 getBlock() {
    
    currentMortonPos ^= mortonPos;
    int n = 0;
    int mask = 0x3f << (MAX_DEPTH-2) * 6;
    while (n < depth && (currentMortonPos & mask) == 0) {
        n++;
        mask >>= 6;
    }

    currentMortonPos = mortonPos;

    if (n < depth)
       depth = n;

    posOffset = (MAX_DEPTH-1 - depth) * 6;

    while (depth < MAX_DEPTH) {
        posOffset -= 6;

        uint index = (mortonPos >> posOffset) & 0x3f;
        u64vec2 node = nodes[stack[depth]];
        
        if ((node.y & 1) == 1) {
            return node.x;

        } else if ((node.x >> index & 1) == 0) {
            return -1;
        }

        stack[++depth] = children_array[
            uint(node.y >> 32)
        ][index];
    }

    return 0;
}
