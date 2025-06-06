#version 460 core
#extension GL_ARB_gpu_shader_int64 : enable
#extension GL_NV_gpu_shader5 : enable

#define u64 uint64_t
#define i64 int64_t
#define u32 uint
#define u8 uint8_t

out layout (location = 0) vec4 FragColor;
out layout (location = 1) vec3 PosOut;
out layout (location = 2) vec3 NormalOut;
//out layout (location = 0) vec4 FragOut;
//out layout (location = 1) vec4 FragOut2;

uniform vec3 origin;
uniform vec3 cameraDir;
uniform vec2 mousePos;
uniform int renderPosData;
uniform uint rootNodeIndex;
uniform uint originMortonPos;
uniform vec2 pixelSize;
uniform vec2 projPlaneSize;
uniform uvec2 resolution;

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

    dvec3 ratiosX;
    dvec3 ratiosY;
    dvec3 ratiosZ;

    dvec3 delta;

    dvec3 absDelta;
};

double ifZeroMakeOne(double n) {
    return n == 0 ? 1 : n;
}

double matchSign(double a, double sign) {
    return a * sign;
}

double makeRatio(double a, double b) {
    if (b == 0) {
        return 0;
    }
    return abs(a) / abs(b);
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

    ray.ratiosX = dvec3(
        ray.step.x,
        makeRatio(dir.y,dir.x) * ray.step.y,
        makeRatio(dir.z,dir.x) * ray.step.z
    );
    ray.ratiosY = dvec3(
        makeRatio(dir.x,dir.y) * ray.step.x,
        ray.step.y,
        makeRatio(dir.z,dir.y) * ray.step.z
    );
    ray.ratiosZ = dvec3(
        makeRatio(dir.x,dir.z) * ray.step.x,
        makeRatio(dir.y,dir.z) * ray.step.y,
        ray.step.z
    );

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

ivec3 lastHit;
uint lastHitFlags = 0;
float lastHitMetadata = 0;

u64vec2 getBlock();
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

vec3 color_int_to_vec3(u64 color) {
    return vec3((color >> (BITS_PER_COLOR * 2)) * SCALED_COLOR,
        ((color >> BITS_PER_COLOR) & COLOR_MASK) * SCALED_COLOR,
        (color & COLOR_MASK) * SCALED_COLOR
    );
}

u64 vec3_to_color_int(vec3 color) {
    return (u64(color.x) << 42) | (u64(color.x) << 21) | u64(color.x);
}

float sigmoid(float x, float scale, float dropOffSteepness) {
    return abs(1.0/(1+exp(-x*dropOffSteepness))*scale);
}

vec3 genSkyBox() {
    if (ray.dir.y < 0) ray.dir.y *= 1.4;
    float haze = (0.1-abs(clamp(ray.dir.y,-.3,.3))) * 0.8 + 0.1;
    float modifier = sigmoid(1-(haze*2),1.0,2.0);
    vec3 hazeMask = vec3(haze);
    vec3 skyDefaultColor = vec3(0.2,0.4,1);
    return (skyDefaultColor + clamp(haze,0,1) * 3) * modifier;
}

vec3 r = vec3(1,0,0);
vec3 g = vec3(0,1,0);
vec3 b = vec3(0,0,1);

vec3 finalColorMod = vec3(1);

void reflectRay(u64vec2 block) {
    pos.deltaPos += ray.absDelta * min(lastHit, 0);
    ray.step *= lastHit;
    ray.dir *= lastHit;
    finalColorMod *= vec3(1) - ((vec3(1) - color_int_to_vec3(block.x)) * (1 - float(block.y >> 32)));
}

void refractRay(u64 color) {

}

void main()
{

    if (renderPosData == 0 && distance(gl_FragCoord.xy, mousePos) <= 3) {
        FragColor = vec4(1,1,1,0);
        return;
    }

    float pixelWidth = 1.0/resolution.x;
    float pixelHeight = 1.0/resolution.y;

    stack[0] = 0;
    depth = 0;

    pos.round = ivec3(floor(origin));

    mortonPos = originMortonPos;

    vec2 FragCoord = gl_FragCoord.xy * vec2(pixelWidth,pixelHeight);//pixelSize;

    vec3 camLeft = cross(cameraDir,vec3(0,1,0));
    vec3 camUp = cross(cameraDir,camLeft);
    vec3 projection_plane_center = cameraDir;
    vec3 projection_plane_left = cross(projection_plane_center, vec3(0,1,0));
    vec3 projection_plane_intersect = normalize(projection_plane_center + 
        (projection_plane_left * -(projPlaneSize.x * (FragCoord.x - 0.5))) + 
        cross(projection_plane_center, projection_plane_left) * (-FragCoord.y + 0.5) * projPlaneSize.y);
    

    ray = buildRay(projection_plane_intersect);

    pos.exact = origin;
    
    if (ray.step.x < 0) pos.exact.x -= 1;
    if (ray.step.y < 0) pos.exact.y -= 1;
    if (ray.step.z < 0) pos.exact.z -= 1;

    pos.deltaPos = ray.absDelta - (pos.exact - pos.round) * ray.delta;

    uint i = 0;

    float currentRefractiveIndex = 1.0;

    u64vec2 block = getBlock();
    if (block.x != -1) {
        if ((block.y & 0x4) == 1) {
            currentRefractiveIndex = float(block.y >> 32);
        } else {
            FragColor = vec4(color_int_to_vec3(block.x),0);
            return;
        }
    }

    const uint numSteps = 200;
    while (i < numSteps) {

        do {
            nextIntersectDDA();
            //nextIntersect(1);
            block = getBlock();
        } while (block.x == -1 && i++ < numSteps);

        if (block.x == -1) break;

        uint flags = uint(block.y) & 0x7;
        if (flags == 0x3) {
            reflectRay(block);

        } else if (flags == 0x5) {
            break;

        } else {
            break;
        }
    }

    if (block.x != -1) {
        FragColor = vec4(color_int_to_vec3(block.x) * finalColorMod, 1);

        vec3 tmp = -min(lastHit,0) * vec3(0,1,2);
        int index = int(tmp.x + tmp.y + tmp.z);

        float val = abs(pos.round[index]-origin[index]);
        if (index == 0)
            PosOut = vec3(val * ray.ratiosX);
        else if (index == 1)
            PosOut = vec3(val * ray.ratiosY);
        else if (index == 2)
            PosOut = vec3(val * ray.ratiosZ);
        
        if (distance(abs(pos.round).z,abs(ivec3(floor(PosOut+origin))).z) > 2) FragColor = vec4(g,0);

        PosOut = pos.exact;
        
        NormalOut = -min(lastHit,0) * ray.step;
    } else {
        FragColor = vec4(genSkyBox() * finalColorMod, 0); 
    }

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

void nextIntersect(int step) {

    vec3 dst = step - abs(fract(pos.exact));

    if (pos.deltaPos.x < pos.deltaPos.y && pos.deltaPos.x < pos.deltaPos.z) {

        pos.exact += vec3(dst.x * ray.ratiosX);
        pos.round.x += ray.step.x * step;
        
        uint n = pos.round.x;
        n = (n | (n << 16)) & 0x030000FF;
        n = (n | (n <<  8)) & 0x0300F00F;
        n = (n | (n <<  4)) & 0x030C30C3;
        mortonPos &= 0xFCF3CF3C;
        mortonPos |= n;

        lastHit = ivec3(-1,1,1);

    } else if (pos.deltaPos.y < pos.deltaPos.z) {
        
        pos.exact += vec3(dst.y * ray.ratiosY);
        pos.round.y += ray.step.y * step;

        uint n = pos.round.y;
        n = (n | (n << 16)) & 0x030000FF;
        n = (n | (n <<  8)) & 0x0300F00F;
        n = (n | (n <<  4)) & 0x030C30C3;
        mortonPos &= 0xFCF3CF3C << 2 | 0x3;
        mortonPos |= n << 2;

        lastHit = ivec3(1,-1,1);

    } else {
        pos.exact += vec3(dst.z * ray.ratiosZ);
        pos.round.z += ray.step.z * step;

        uint n = pos.round.z;
        n = (n | (n << 16)) & 0x030000FF;
        n = (n | (n <<  8)) & 0x0300F00F;
        n = (n | (n <<  4)) & 0x030C30C3;
        mortonPos &= 0xFCF3CF3C << 4 | 0xf;
        mortonPos |= n << 4;

        lastHit = ivec3(1,1,-1);
    }

    pos.deltaPos = ray.absDelta - fract(pos.exact) * ray.delta;
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
