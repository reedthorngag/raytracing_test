#version 460 core
#extension GL_ARB_gpu_shader_int64 : enable
#extension GL_NV_gpu_shader5 : enable

#define u64 uint64_t
#define i64 int64_t
#define u32 uint
#define u8 uint8_t

uniform uvec2 resolution;
uniform vec3 origin;

uniform layout (binding = 0) sampler2D fragColor;
uniform layout (binding = 1) sampler2D startPoint;
uniform layout (binding = 2) sampler2D normal;

out vec3 FragColor;

layout (std430, binding = 3) readonly buffer layoutNodes {
    u64vec2 nodes[];
};

layout (packed, binding = 2) readonly buffer layoutArrays {
    uint[64] children_array[];
};

vec3 r = vec3(1,0,0);
vec3 g = vec3(0,1,0);
vec3 b = vec3(0,0,1);

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
uint currentMortonPos = 0;
int depth;
uint mortonPos = 0;

uint originMortonPos;

ivec3 lastHit;
uint lastHitFlags = 0;
float lastHitMetadata = 0;

u64vec2 getBlock();
void nextIntersectDDA();

const int BITS_PER_COLOR = 21;
const int COLOR_RANGE = 1 << BITS_PER_COLOR;
const int COLOR_MASK = COLOR_RANGE - 1;
const double SCALED_COLOR = 1.0 / COLOR_RANGE;

vec3 color_int_to_vec3(u64 color) {
    return vec3((color >> (BITS_PER_COLOR * 2)) * SCALED_COLOR,
        ((color >> BITS_PER_COLOR) & COLOR_MASK) * SCALED_COLOR,
        (color & COLOR_MASK) * SCALED_COLOR
    );
}

u64 vec3_to_color_int(vec3 color) {
    return (u64(color.x) << 42) | (u64(color.x) << 21) | u64(color.x);
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

vec3[] hemisphereDirs = {
    vec3(0.07430756460710652,0.97875,-0.19111991874778692),
    vec3(-0.3150707529400007,0.93625,0.15545532522825017),
    vec3(0.43068766605847436,0.89375,0.12537572255067567),
    vec3(-0.27244438490890477,0.8512500000000001,-0.4484946985546301),
    vec3(-0.11439109511443586,0.8087500000000001,0.5769212380026584),
    vec3(0.517891665482911,0.76625,-0.380327701230579),
    vec3(-0.6860266781979666,0.7237500000000001,-0.07452069712947905),
    vec3(0.48323377346512497,0.68125,0.5498941331589711),
    vec3(0.015898658644648525,0.63875,-0.769250069972893),
    vec3(-0.5543988637067352,0.5962500000000001,0.5806271070322852),
    vec3(0.8308218084042563,0.5537500000000001,-0.05564225175063546),
    vec3(-0.6710951947823731,0.51125,-0.5368935434888453),
    vec3(0.13585390932049551,0.46875000000000006,0.8728213750947776),
    vec3(0.5011950375796783,0.4262499999999999,-0.7530700311428579),
    vec3(-0.8965402339742274,0.38375000000000004,0.22124996353770887),
    vec3(0.8250469005357429,0.3412500000000001,0.4503843352253316),
    vec3(-0.3087795461729986,0.29875,-0.9029970262216798),
    vec3(-0.38720001271637,0.25625000000000003,0.8856704170584241),
    vec3(0.8931680415940547,0.2137499999999999,-0.39567889376998755),
    vec3(-0.9337752751575846,0.17125,-0.31422471736700947),
};

vec3[] ratioMap = {
    vec3(1,ray.ratioXtoY,ray.ratioXtoZ),
    vec3(ray.ratioYtoX,1,ray.ratioYtoZ),
    vec3(ray.ratioZtoX,ray.ratioZtoY,1)
};

vec3 finalColorMod = vec3(1);

void reflectRay(u64vec2 block) {
    pos.deltaPos += ray.absDelta * min(lastHit, 0);
    ray.step *= lastHit;
    ray.dir *= lastHit;
    finalColorMod *= vec3(1) - ((vec3(1) - color_int_to_vec3(block.x)) * (1 - float(block.y >> 32)));
}

void refractRay(u64 color) {

}

u64 castRay(vec3 dir, vec3 startPos, uint maxSteps) {

    ray = buildRay(dir);
    pos.exact = startPos;
    pos.round = ivec3(floor(startPos));

    if (ray.step.x < 0) pos.exact.x -= 1;
    if (ray.step.y < 0) pos.exact.y -= 1;
    if (ray.step.z < 0) pos.exact.z -= 1;

    pos.deltaPos.x = ray.absDelta.x - (pos.exact.x - pos.round.x) * ray.delta.x;
    pos.deltaPos.y = ray.absDelta.y - (pos.exact.y - pos.round.y) * ray.delta.y;
    pos.deltaPos.z = ray.absDelta.z - (pos.exact.z - pos.round.z) * ray.delta.z;

    float currentRefractiveIndex = 1.0;

    u64vec2 block;
    uint i = 0;

    while (i < maxSteps) {

        do {
            nextIntersectDDA();
            block = getBlock();
        } while (block.x == -1 && i++ < maxSteps);


        return block.x;
        // if (block.x == -1) {
        //     return -1;
        // }

        // uint flags = uint(block.y) & 0x7;
        // if (flags == 0x3) {
        //     reflectRay(block);

        // } else if (flags == 0x5) {
        //     break;

        // } else {
        //     return block.x;
        // }
    }

    return -1;
}

vec3 raycastHemisphere(vec3 color, vec3 startPos, vec3 normal) {

    mat3 transformMat = computeTransformMat(normal);

    const int numRays = 1;

    for (int i = 0; i < numRays; i++) {

        mortonPos = originMortonPos;
        u64 color2 = castRay(vec3(0,0,1), startPos, 5);
        if (color2 != -1) color = b;
    }

    return color;
}

void main() {

    vec2 fragCoord = gl_FragCoord.xy * (1.0/resolution);

    vec4 color = texture(fragColor, fragCoord);
    if (color.w == 0) {
        FragColor = color.xyz;
        return;
    }

    vec3 startPos = origin;//texture(startPoint,fragCoord).xyz;

    stack[0] = 0;
    depth = 0;

    ivec3 pos = ivec3(floor(startPos));

    int n = pos.x;
    n = (n | (n << 16)) & 0x030000FF;
    n = (n | (n <<  8)) & 0x0300F00F;
    n = (n | (n <<  4)) & 0x030C30C3;
    int x = n;

    n = pos.y;
    n = (n | (n << 16)) & 0x030000FF;
    n = (n | (n <<  8)) & 0x0300F00F;
    n = (n | (n <<  4)) & 0x030C30C3;
    int y = n;

    n = pos.z;
    n = (n | (n << 16)) & 0x030000FF;
    n = (n | (n <<  8)) & 0x0300F00F;
    n = (n | (n <<  4)) & 0x030C30C3;

    mortonPos = (n << 4) | (y << 2) | x;
    originMortonPos = mortonPos;

    // ray = buildRay()

    // u64vec2 block;

    // for (int i=0; i < 50; i++) {
    //     nextIntersectDDA();
    //     block = getBlock();
    //     if (block.x != -1) {
    //         FragColor = r;
    //         return;
    //     }
    // }
    // FragColor = b;
    // return;

    FragColor = raycastHemisphere(color.xyz, startPos, texture(normal,fragCoord).xyz);
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

        lastHit = ivec3(-1,1,1);

    } else if (pos.deltaPos.y < pos.deltaPos.z) {
        pos.round.y += ray.step.y;
        pos.deltaPos.y += ray.absDelta.y;

        uint n = pos.round.y;
        n = (n | (n << 16)) & 0x030000FF;
        n = (n | (n <<  8)) & 0x0300F00F;
        n = (n | (n <<  4)) & 0x030C30C3;
        mortonPos &= 0xFCF3CF3C << 2 | 0x3;
        mortonPos |= n << 2;

        lastHit = ivec3(1,-1,1);

    } else {
        pos.round.z += ray.step.z;
        pos.deltaPos.z += ray.absDelta.z;
        
        uint n = pos.round.z;
        n = (n | (n << 16)) & 0x030000FF;
        n = (n | (n <<  8)) & 0x0300F00F;
        n = (n | (n <<  4)) & 0x030C30C3;
        mortonPos &= 0xFCF3CF3C << 4 | 0xf;
        mortonPos |= n << 4;

        lastHit = ivec3(1,1,-1);
    }
}

u64vec2 getBlock() {
    
    currentMortonPos ^= mortonPos;
    int n = MAX_DEPTH-1;
    while (currentMortonPos != 0) {
        n--;
        currentMortonPos >>= 6;
    }

    currentMortonPos = mortonPos;

    depth = n < depth ? n : depth;

    uint posOffset = (MAX_DEPTH-1 - depth) * 6;

    while (depth < MAX_DEPTH) {

        u64vec2 node = nodes[stack[depth]];
        
        uint a = uint(node.y & 1);
        if (a == 1) {
            return node;
        }

        posOffset -= 6;
        uint index = (mortonPos >> posOffset) & 0x3f;

        a = uint(node.x >> index & 1);
        if (a == 0) {
            return u64vec2(-1,0);
        }

        stack[++depth] = children_array[
            uint(node.y >> 32)
        ][index];
    }

    return u64vec2(0,0);
}

