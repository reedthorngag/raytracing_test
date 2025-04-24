#version 460 core
#extension GL_ARB_gpu_shader_int64 : enable
#extension GL_NV_gpu_shader5 : enable

#define u64 uint64_t
#define i64 int64_t
#define u32 uint
#define u8 uint8_t

out vec4 FragColor;

uniform vec3 origin;
uniform vec3 cameraDir;
uniform vec2 mousePos;
uniform int renderPosData;
uniform uint rootNodeIndex;

uniform sampler2D startPoints;

in vec4 gl_FragCoord;

layout (std430, binding = 3) buffer layoutNodes {
    u64vec2 nodes[];
};

layout (packed, binding = 2) buffer layoutArrays {
    uint[64] children_array[];
};


int width = 1920;
int height = 1080;
int sourceWidth = width >> 2;
int sourceHeight = height >> 2;

float aspect_ratio = float(height) / width;

float pixelWidth = 1.0/width;
float pixelHeight = 1.0/height;
float sourcePixelWidth = 1.0/sourceWidth;
float sourcePixelHeight = 1.0/sourceHeight;

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

    // ray.ratioYtoX = matchSign(makeRatio(dir.y,dir.x),ray.step.y);
    // ray.ratioYtoZ = matchSign(makeRatio(dir.y,dir.z),ray.step.y);
    // ray.ratioXtoY = matchSign(makeRatio(dir.x,dir.y),ray.step.x);
    // ray.ratioXtoZ = matchSign(makeRatio(dir.x,dir.z),ray.step.x);
    // ray.ratioZtoX = matchSign(makeRatio(dir.z,dir.x),ray.step.z);
    // ray.ratioZtoY = matchSign(makeRatio(dir.z,dir.y),ray.step.z);

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

u64 getBlock();
void nextIntersect(int step);
void nextIntersectDDA();
void genSkyBox();

vec4 r = vec4(1,0,0,1);
vec4 g = vec4(0,1,0,1);
vec4 b = vec4(0,0,1,1);

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

void setFragToVec(vec3 vec) {
    FragColor = vec4(vec.xyz,0);
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

uint mortonPos = 0;

vec2[] checkPosOffsets = {
    vec2(sourcePixelWidth,0),
    vec2(-sourcePixelWidth,0),
    vec2(sourcePixelHeight,0),
    vec2(-sourcePixelHeight,0),
    vec2(sourcePixelWidth,sourceHeight),
    vec2(-sourcePixelWidth,sourceHeight),
    vec2(sourcePixelWidth,-sourceHeight),
    vec2(-sourcePixelWidth,-sourceHeight),
};

void main() {

    if (renderPosData == 0 && distance(gl_FragCoord.xy, mousePos) < 3) {
        FragColor = vec4(1,1,1,1);
        return;
    }

    stack[0] = 0;
    depth = 0;

    vec2 FragCoord = vec2(
        gl_FragCoord.x * pixelWidth,
        gl_FragCoord.y * pixelHeight
    );

    vec3 camLeft = cross(cameraDir,vec3(0,1,0));
    vec3 camUp = cross(cameraDir,camLeft);
    vec3 projection_plane_center = cameraDir;
    vec3 projection_plane_left = normalize(cross(projection_plane_center, vec3(0,1,0)));
    vec3 projection_plane_intersect = normalize(projection_plane_center + 
        (projection_plane_left * -(projection_plane_width * (FragCoord.x - 0.5))) + 
        cross(projection_plane_center, projection_plane_left) * (-FragCoord.y + 0.5) * projection_plane_height);
    

    ray = buildRay(projection_plane_intersect);

    vec4 sourceRay = texture(startPoints,FragCoord);

    for (int i = 0; i < checkPosOffsets.length(); i++) {

        vec4 tex = texture(startPoints,FragCoord + checkPosOffsets[i]);
        if (tex.w < sourceRay.w) sourceRay = tex;
    }

    pos.exact = origin + sourceRay.xyz*ray.dir;
    pos.round = ivec3(floor(pos.exact));

    int n = pos.round.x;
    n = (n | (n << 16)) & 0x030000FF;
    n = (n | (n <<  8)) & 0x0300F00F;
    n = (n | (n <<  4)) & 0x030C30C3;
    int x = n;

    n = pos.round.y;
    n = (n | (n << 16)) & 0x030000FF;
    n = (n | (n <<  8)) & 0x0300F00F;
    n = (n | (n <<  4)) & 0x030C30C3;
    int y = n;

    n = pos.round.z;
    n = (n | (n << 16)) & 0x030000FF;
    n = (n | (n <<  8)) & 0x0300F00F;
    n = (n | (n <<  4)) & 0x030C30C3;

    mortonPos = (n << 4) | (y << 2) | x;
    currentMortonPos = mortonPos;

    u64 color = getBlock();
    if (color != -1) {
        FragColor = color_int_to_vec4(color);
        return;
    }
    
    if (ray.step.x < 0) pos.exact.x -= 1;
    if (ray.step.y < 0) pos.exact.y -= 1;
    if (ray.step.z < 0) pos.exact.z -= 1;

    int step = 1;

    pos.deltaPos.x = ray.absDelta.x - (pos.exact.x - pos.round.x) * ray.delta.x;
    pos.deltaPos.y = ray.absDelta.y - (pos.exact.y - pos.round.y) * ray.delta.y;
    pos.deltaPos.z = ray.absDelta.z - (pos.exact.z - pos.round.z) * ray.delta.z;

    bool set = false;
    for (int i = 0; i < 170; i++) {

        nextIntersectDDA();

        color = getBlock();
        if (color != -1) {
            FragColor = color_int_to_vec4(color);
            set = true;
            break;
        }
    }

    if (!set) {
        genSkyBox();
    }

    { // debug data
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
        else if (renderPosData == 9) {
            FragColor = vec4(
                int(bitCount(int(nodes[0].y))),
                int(bitCount(int(nodes[0].x >> 32))),
                int(bitCount(int(nodes[0].x))),
                int(bitCount(int(nodes[0].y >> 32)))
            );
        } else if (renderPosData == 10)
            setFragToVec(vec3(stack[3],stack[4],stack[5]));
        
    }
}

float sigmoid(float x, float scale, float dropOffSteepness) {
    return abs(1.0/(1+exp(-x*dropOffSteepness))*scale);
}

void genSkyBox() {
    if (ray.dir.y < 0) ray.dir.y *= 1.4;
    float haze = (0.1-abs(clamp(ray.dir.y,-.3,.3))) * 0.8 + 0.1;
    float modifier = sigmoid(1-(haze*2),1.0,2.0);
    vec3 hazeMask = vec3(haze);
    vec3 skyDefaultColor = vec3(0.2,0.4,1);
    FragColor = vec4((skyDefaultColor + clamp(haze,0,1) * 3)*modifier,1);
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

    } else if (pos.deltaPos.y < pos.deltaPos.z) {
        pos.round.y += ray.step.y;
        pos.deltaPos.y += ray.absDelta.y;

        uint n = pos.round.y;
        n = (n | (n << 16)) & 0x030000FF;
        n = (n | (n <<  8)) & 0x0300F00F;
        n = (n | (n <<  4)) & 0x030C30C3;
        mortonPos &= 0xFCF3CF3C << 2 | 0x3;
        mortonPos |= n << 2;

    } else {
        pos.round.z += ray.step.z;
        pos.deltaPos.z += ray.absDelta.z;
        
        uint n = pos.round.z;
        n = (n | (n << 16)) & 0x030000FF;
        n = (n | (n <<  8)) & 0x0300F00F;
        n = (n | (n <<  4)) & 0x030C30C3;
        mortonPos &= 0xFCF3CF3C << 4 | 0xf;
        mortonPos |= n << 4;

    }
}

// need to work out how to update pos.deltaPos more efficiently if possible
// void nextIntersect(int step) {

//     ivec3 steps = ray.step * step;

//     float xDst = (pos.round.x + steps.x) - pos.exact.x;
//     float yDst = (pos.round.y + steps.y) - pos.exact.y;
//     float zDst = (pos.round.z + steps.z) - pos.exact.z;

//     if (pos.deltaPos.x < pos.deltaPos.y && pos.deltaPos.x < pos.deltaPos.z) {

//         pos.exact.x += xDst;
//         pos.round.x += steps.x;
//         xDst = abs(xDst);
//         pos.exact.y += xDst * ray.ratioYtoX;
//         pos.exact.z += xDst * ray.ratioZtoX;

//     } else if (pos.deltaPos.y < pos.deltaPos.z) {
        
//         pos.exact.y += yDst;
//         pos.round.y += steps.y;
//         yDst = abs(yDst);
//         pos.exact.x += yDst * ray.ratioXtoY;
//         pos.exact.z += yDst * ray.ratioZtoY;

//     } else {
//         pos.exact.z += zDst;
//         pos.round.z += steps.z;
//         zDst = abs(zDst);
//         pos.exact.x += zDst * ray.ratioXtoZ;
//         pos.exact.y += zDst * ray.ratioYtoZ;
//     }

//     pos.deltaPos.x = ray.absDelta.x - (pos.exact.x - pos.round.x) * ray.delta.x;
//     pos.deltaPos.y = ray.absDelta.y - (pos.exact.y - pos.round.y) * ray.delta.y;
//     pos.deltaPos.z = ray.absDelta.z - (pos.exact.z - pos.round.z) * ray.delta.z;
// }

int[] divideBy6Lookup = {
    0,0,0,0,0,0,
    1,1,1,1,1,1,
    2,2,2,2,2,2,
    3,3,3,3,3,3,
    4,4,4,4,4,4,
    5,5,5,5,5,5,
};

int findMSBb(int n) {
    for (int i = 31; i >= 0; i--)
        if (((n >> i) & 1) == 1) return i;
    return -1;
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
