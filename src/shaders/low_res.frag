#version 460 core
#extension GL_ARB_gpu_shader_int64 : enable
#extension GL_NV_gpu_shader5 : enable

#define u64 uint64_t
#define i64 int64_t
#define u32 uint
#define u8 uint8_t

out layout (location = 0) vec4 FragOut;
out layout (location = 1) vec4 FragOut2;

uniform vec3 origin;
uniform vec3 cameraDir;
uniform vec2 mousePos;
uniform int renderPosData;
uniform uint rootNodeIndex;
uniform uint originMortonPos;

in vec4 gl_FragCoord;

layout (std430, binding = 3) readonly buffer layoutNodes {
    u64vec2 nodes[];
};

layout (packed, binding = 2) readonly buffer layoutArrays {
    uint[64] children_array[];
};


int width = 1920 >> 2;
int height = 1080 >> 2;

float aspect_ratio = float(height) / width;

double pixelWidth = 1.0/width;
double pixelHeight = 1.0/height;

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


uint mortonPos = 0;

void main()
{

    stack[0] = 0;
    depth = 0;

    pos.round = ivec3(floor(origin));

    mortonPos = originMortonPos;
    currentMortonPos = mortonPos;

    if (getBlock() != -1) {
        FragOut = vec4(0);
        return;
    }

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

        if (getBlock() != -1) {
            break;
        }
    }

    if (pos.deltaPos.x < pos.deltaPos.y && pos.deltaPos.x < pos.deltaPos.z) {
        pos.round.x += ray.step.x;
        pos.exact = vec3(pos.round.x,ray.ratioXtoY,ray.ratioXtoZ);
    } else if (pos.deltaPos.y < pos.deltaPos.z) {
        pos.round.y += ray.step.y;
        pos.exact = vec3(ray.ratioYtoX,pos.round.y,ray.ratioYtoZ);
    } else {
        pos.round.z += ray.step.z;
        pos.exact = vec3(ray.ratioZtoX,ray.ratioZtoY,pos.round.z);
    }
    pos.exact -= ray.dir;
    FragOut = vec4(abs(pos.exact-origin), distance(pos.exact,origin));
    FragOut2 = vec4(ray.dir,0);
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
