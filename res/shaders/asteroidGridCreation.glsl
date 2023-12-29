#version 430

layout(local_size_x = 1024, local_size_y = 1, local_size_z = 1) in;

struct AsteroidData
{
    vec4 position;
    vec4 rotation;
    vec4 scale;
    vec4 velocity;
    vec4 angularVelocity;
};

struct CellData {
    int spatialLookup[2];
    int startIndices;
};

struct CellCord {
    int x;
    int y;
    int z;
};

layout (std430, binding = 0) buffer AsteroidBuffer {
    AsteroidData asteroidsData[];
};

layout (std430, binding = 1) buffer CellBuffer {
    CellData cellData[];
};

CellCord PositionToCellCoord(vec3 position, float radius){
    CellCord r;
    r.x = int(position.x/radius);
    r.y = int(position.y/radius);
    r.z = int(position.z/radius);
    return r;
}

uint HashCell(CellCord cellCord){
    uint a = uint(cellCord.x * 9737339);
    uint c = uint(cellCord.y * 9737341);
    uint b = uint(cellCord.z * 9737333);
    return (a + b + c) % uint(cellData.length());
}

uniform float collisonRadius;

void main() {
    uint index = gl_GlobalInvocationID.x;
    cellData[index].spatialLookup[0] = int(gl_GlobalInvocationID.x);
    cellData[index].spatialLookup[1] = int(HashCell(PositionToCellCoord(asteroidsData[gl_GlobalInvocationID.x].position.xyz,collisonRadius)));
}