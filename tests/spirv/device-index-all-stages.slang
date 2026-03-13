//TEST:SIMPLE(filecheck=VERT): -target spirv -entry vertMain -stage vertex -skip-spirv-validation
//TEST:SIMPLE(filecheck=FRAG): -target spirv -entry fragMain -stage fragment -skip-spirv-validation
//TEST:SIMPLE(filecheck=GEOM): -target spirv -entry geomMain -stage geometry -skip-spirv-validation
//TEST:SIMPLE(filecheck=HULL): -target spirv -entry hullMain -stage hull -skip-spirv-validation
//TEST:SIMPLE(filecheck=DOMAIN): -target spirv -entry domainMain -stage domain -skip-spirv-validation
//TEST:SIMPLE(filecheck=COMP): -target spirv -entry compMain -stage compute -skip-spirv-validation
//TEST:SIMPLE(filecheck=MESH): -target spirv -emit-spirv-directly -skip-spirv-validation -entry meshMain
//TEST:SIMPLE(filecheck=AMPL): -target spirv -emit-spirv-directly -skip-spirv-validation -entry ampMain

// Test that SV_DeviceIndex is valid as input in all shader stages (per SPIR-V spec).

// VERT: OpDecorate %{{.*}} BuiltIn DeviceIndex
[shader("vertex")]
float4 vertMain(uint deviceIdx : SV_DeviceIndex) : SV_Position
{
    return float4(float(deviceIdx), 0, 0, 1);
}

// FRAG: OpDecorate %{{.*}} BuiltIn DeviceIndex
[shader("fragment")]
float4 fragMain(uint deviceIdx : SV_DeviceIndex) : SV_Target
{
    return float4(float(deviceIdx), 0, 0, 1);
}

struct GeomVertex
{
    float4 pos : SV_Position;
};

// GEOM: OpDecorate %{{.*}} BuiltIn DeviceIndex
[maxvertexcount(3)]
[shader("geometry")]
void geomMain(
    triangle GeomVertex input[3],
    uint deviceIdx : SV_DeviceIndex,
    inout TriangleStream<GeomVertex> stream)
{
    GeomVertex v;
    v.pos = input[0].pos + float(deviceIdx);
    stream.Append(v);
}

struct HsIn
{
    float3 position : POSITION;
};

struct HsOut
{
    float3 position : POSITION;
};

struct HscOut
{
    float EdgeTessFactor[4] : SV_TessFactor;
    float InsideTessFactor[2] : SV_InsideTessFactor;
};

struct DsOut
{
    float4 position : SV_Position;
};

HscOut hullConstants(InputPatch<HsIn, 4> patch)
{
    HscOut o;
    o.EdgeTessFactor[0] = 1.0;
    o.EdgeTessFactor[1] = 1.0;
    o.EdgeTessFactor[2] = 1.0;
    o.EdgeTessFactor[3] = 1.0;
    o.InsideTessFactor[0] = 1.0;
    o.InsideTessFactor[1] = 1.0;
    return o;
}

// HULL: OpDecorate %{{.*}} BuiltIn DeviceIndex
[domain("quad")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(4)]
[patchconstantfunc("hullConstants")]
HsOut hullMain(InputPatch<HsIn, 4> patch, uint i : SV_OutputControlPointID, uint deviceIdx : SV_DeviceIndex)
{
    HsOut o;
    o.position = patch[i].position + float(deviceIdx);
    return o;
}

// DOMAIN: OpDecorate %{{.*}} BuiltIn DeviceIndex
[domain("quad")]
DsOut domainMain(
    float2 uv : SV_DomainLocation,
    const OutputPatch<HsOut, 4> patch,
    const HscOut patchConstants,
    uint deviceIdx : SV_DeviceIndex)
{
    DsOut o;
    float3 pos = patch[0].position * (1 - uv.x) * (1 - uv.y)
               + patch[1].position * uv.x * (1 - uv.y)
               + patch[2].position * (1 - uv.x) * uv.y
               + patch[3].position * uv.x * uv.y;
    o.position = float4(pos + float(deviceIdx), 1.0);
    return o;
}

RWStructuredBuffer<uint> outputBuffer;

// COMP: OpDecorate %{{.*}} BuiltIn DeviceIndex
[shader("compute")]
[numthreads(1, 1, 1)]
void compMain(uint deviceIdx : SV_DeviceIndex)
{
    outputBuffer[0] = deviceIdx;
}

struct MeshVertex
{
    float4 pos : SV_Position;
};

// MESH: OpDecorate %{{.*}} BuiltIn DeviceIndex
[shader("mesh")]
[numthreads(1, 1, 1)]
[outputtopology("triangle")]
void meshMain(
    uint deviceIdx : SV_DeviceIndex,
    OutputVertices<MeshVertex, 3> verts,
    OutputIndices<uint3, 1> indices)
{
    SetMeshOutputCounts(3, 1);
    verts[0].pos = float4(float(deviceIdx), 0, 0, 1);
    verts[1].pos = float4(1, 0, 0, 1);
    verts[2].pos = float4(0, 1, 0, 1);
    indices[0] = uint3(0, 1, 2);
}

// AMPL: OpDecorate %{{.*}} BuiltIn DeviceIndex
struct Payload
{
    uint data;
};

[shader("amplification")]
[numthreads(1, 1, 1)]
void ampMain(uint deviceIdx : SV_DeviceIndex)
{
    Payload p;
    p.data = deviceIdx;
    DispatchMesh(1, 1, 1, p);
}
