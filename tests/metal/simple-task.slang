//TEST:SIMPLE(filecheck=CHECK): -target metal
//TEST:SIMPLE(filecheck=CHECK-ASM): -target metallib
//TEST:REFLECTION(filecheck=REFLECT):-target metal -entry taskMain -stage task

// TEST_INPUT: ubuffer(data=[0 0 0 0], stride=4):out,name outputBuffer

uniform RWStructuredBuffer<float> outputBuffer;

cbuffer Uniforms
{
    float4x4 modelViewProjection;
}

// CHECK-ASM: define void @taskMain

//
// Task shader
//

struct MeshPayload
{
    int exponent;
};

// CHECK: MeshPayload_0 object_data& _slang_mesh_payload [[payload]]
// CHECK: mesh_grid_properties  _slang_mgp
[numthreads(1,1,1)]
void taskMain()
{
    // CHECK: _slang_mgp.set_threadgroups_per_grid
    // CHECK: _slang_mesh_payload
    MeshPayload p;
    p.exponent = 3;
    DispatchMesh(1, 1, 1, p);
}

//
// Mesh shader
//

const static float2 positions[3] = {
    float2(0.0, -0.5),
    float2(0.5, 0.5),
    float2(-0.5, 0.5)
};

const static float3 colors[3] = {
    float3(1.0, 1.0, 0.0),
    float3(0.0, 1.0, 1.0),
    float3(1.0, 0.0, 1.0)
};

struct Vertex
{
    float4 pos : SV_Position;
    float3 color : Color;
    int index : Index;
    int value : Value;
};

const static uint MAX_VERTS = 12;
const static uint MAX_PRIMS = 4;

[outputtopology("triangle")]
[numthreads(12, 1, 1)]
void meshMain(
    in uint tig: SV_GroupIndex,
    in payload MeshPayload meshPayload,
    // Check that we correctly generate the specific 'in payload' that HLSL
    // requires:
    // HLSL: , in payload MeshPayload
    OutputVertices<Vertex, MAX_VERTS> verts,
    OutputIndices<uint3, MAX_PRIMS> triangles)
{
    const uint numVertices = 12;
    const uint numPrimitives = 4;
    SetMeshOutputCounts(numVertices, numPrimitives);

    if (tig < numVertices)
    {
        const int tri = tig / 3;
        verts[tig] = { float4(positions[tig % 3], 0, 1), colors[tig % 3], tri, int(pow(tri, meshPayload.exponent)) };
    }

    if (tig < numPrimitives)
        triangles[tig] = tig * 3 + uint3(0, 1, 2);
}

//
// Fragment Shader
//

struct Fragment
{
    float4 color : SV_Target;
};

Fragment fragmentMain(Vertex input)
{
    outputBuffer[input.index] = input.value;

    Fragment output;
    output.color = float4(input.color, 1.0);
    return output;
}

