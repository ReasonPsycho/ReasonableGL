#version 430

layout(local_size_x = 27, local_size_y = 1, local_size_z = 1) in;

struct AsteroidData {
    vec4 position;
    vec4 rotation;
    vec4 scale;
    vec4 velocity;
    vec4 angularVelocity;
    vec4 separationVector;
    vec4 changeInVelocity;
    vec4 changeInAngularVelocity;
};

const ivec3 offsets3D[27] =
{
ivec3(-1, -1, -1),
ivec3(-1, -1, 0),
ivec3(-1, -1, 1),
ivec3(-1, 0, -1),
ivec3(-1, 0, 0),
ivec3(-1, 0, 1),
ivec3(-1, 1, -1),
ivec3(-1, 1, 0),
ivec3(-1, 1, 1),
ivec3(0, -1, -1),
ivec3(0, -1, 0),
ivec3(0, -1, 1),
ivec3(0, 0, -1),
ivec3(0, 0, 0),
ivec3(0, 0, 1),
ivec3(0, 1, -1),
ivec3(0, 1, 0),
ivec3(0, 1, 1),
ivec3(1, -1, -1),
ivec3(1, -1, 0),
ivec3(1, -1, 1),
ivec3(1, 0, -1),
ivec3(1, 0, 0),
ivec3(1, 0, 1),
ivec3(1, 1, -1),
ivec3(1, 1, 0),
ivec3(1, 1, 1)
};

struct CellData {
    int key;
    int cellHash;
};

struct Offsets {
    int value;
};

layout(std430, binding = 0) buffer AsteroidBuffer {
    AsteroidData asteroidsData[];
};

layout(std430, binding = 1) buffer CellBuffer {
    CellData cellData[];
};

layout(std430, binding = 2) buffer OffsetsBuffer {
    Offsets offsets[];
};

uvec3 PositionToCellCoord(vec3 position, float radius) {
    return ivec3(floor(position / radius));
}

uint HashCell(uvec3 cellCord){
    uint a = uint(cellCord.x * 15823);
    uint c = uint(cellCord.y * 9737333);
    uint b = uint(cellCord.z * 440817757);
    return (a + b + c) % uint(cellData.length());
}

vec3 SeparationVector(vec3 a, vec3 b, float ra, float rb){
    return ((a - b)/length(a - b)) * (ra + rb  - length(a-b));
}

void CalculateCollisionResponse(in vec3 n,in vec3 v1, in float m1, in vec3 v2, in float m2,
out vec3 v1Final, out vec3 deltaAngularVelocity)
{
    vec3 v_rel = v1 - v2;
    vec3 k = (2*v_rel * n)/((1/m1-1/m2) * n * n);
    v1Final = k * n/m2;
    deltaAngularVelocity =vec3(0);
}

float MeanOfScales(vec3 scale) {
    return dot(scale, vec3(1.0)) / 3.0;
}

float MaxOfScales(vec3 scale) {
    return max(max(scale.x,scale.y),scale.z);
}

uniform float collisionRadius;
uniform float gridRadius;

void main() {
    uint index = gl_WorkGroupID.x;
    vec3 sumOfSeperationVectors = vec3(0);
    vec3 sumOfMovementVectors = vec3(0);
    vec3 sumOfAnagularMovementVectors = vec3(0);
    int amountOfCollisions = 0;
    AsteroidData currentAsteroid = asteroidsData[index];
    float maxScale = MaxOfScales(currentAsteroid.scale.xyz);
    uvec3 cellCoord = PositionToCellCoord(currentAsteroid.position.xyz, gridRadius) + offsets3D[gl_LocalInvocationID.x];
    uint hash = HashCell(cellCoord);
    int currentTablePos = offsets[hash].value;

    while (currentTablePos < int(cellData.length()) && cellData[currentTablePos].cellHash == int(hash)) {
        //if (cellData[currentTablePos].key != int(index)) { We aleready kinda ignore case when points are the same
        AsteroidData asteroidToCheck = asteroidsData[cellData[currentTablePos].key];
        float maxScale2 = MaxOfScales(asteroidToCheck.scale.xyz);

        float dist = length(asteroidToCheck.position.xyz - currentAsteroid.position.xyz);
        if (dist <= collisionRadius * maxScale + collisionRadius * maxScale2) {
            float meanScale = MeanOfScales(currentAsteroid.scale.xyz);
            float meanScale2 = MeanOfScales(asteroidToCheck.scale.xyz);
            vec3 v1Final = vec3(0);
            vec3 deltaAngularVelocity = vec3(0);
            if (dist != 0){
                vec3 currentSeparationVector =  SeparationVector(asteroidToCheck.position.xyz, currentAsteroid.position.xyz, collisionRadius * maxScale, collisionRadius * maxScale2);
                CalculateCollisionResponse(currentSeparationVector,
                asteroidToCheck.velocity.xyz,
                meanScale,
                currentAsteroid.velocity.xyz,
                meanScale2,
                v1Final,
                deltaAngularVelocity);
                sumOfSeperationVectors += currentSeparationVector;
                sumOfMovementVectors += v1Final;
                sumOfAnagularMovementVectors += deltaAngularVelocity;
                amountOfCollisions++;
            }
          
        }
        currentTablePos++;
    }


    if (amountOfCollisions != 0) {
        sumOfSeperationVectors /= float(amountOfCollisions);
        asteroidsData[index].separationVector += vec4(-sumOfSeperationVectors, 1);
        sumOfAnagularMovementVectors /= float(amountOfCollisions);
        asteroidsData[index].changeInVelocity += vec4(-sumOfAnagularMovementVectors, 1);
        sumOfMovementVectors /= float(amountOfCollisions);
        asteroidsData[index].separationVector += vec4(-sumOfMovementVectors, 1);
    }
}
