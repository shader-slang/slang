// Positive-path test for #11303 / #11310: `WaveGetWaveIndex()` and
// `WaveGetNumWaves()` are now permitted on the `_mesh` and
// `_amplification` SPIR-V execution models in addition to `compute`,
// matching VUID-SubgroupId-SubgroupId-04367 / VUID-NumSubgroups-04293.
//
// A regression that re-tightens the SPIR-V `case spirv:` arms in
// `source/slang/hlsl.meta.slang` to `case compute:` only would silently
// change emitted code; this test pins the emitted ops so that change
// would fail CI.

//TEST:SIMPLE(filecheck=CHECKMESH): -target spirv -entry meshMain -stage mesh -emit-spirv-directly
//TEST:SIMPLE(filecheck=CHECKAMP):  -target spirv -entry ampMain  -stage amplification -emit-spirv-directly

const static uint MAX_VERTS = 3;
const static uint MAX_PRIMS = 1;

struct Vertex
{
    float4 pos : SV_Position;
};

struct Primitive
{
    [[vk::location(0)]] uint waveIndex;
    [[vk::location(1)]] uint numWaves;
};

// CHECKMESH-DAG: OpDecorate {{.*}} BuiltIn SubgroupId
// CHECKMESH-DAG: OpDecorate {{.*}} BuiltIn NumSubgroups
[outputtopology("triangle")]
[numthreads(MAX_VERTS, 1, 1)]
void meshMain(
    in uint tig : SV_GroupThreadID,
    OutputVertices<Vertex, MAX_VERTS> verts,
    OutputIndices<uint3, MAX_PRIMS> triangles,
    OutputPrimitives<Primitive, MAX_PRIMS> primitives)
{
    SetMeshOutputCounts(MAX_VERTS, MAX_PRIMS);

    if (tig < MAX_VERTS)
        verts[tig] = { float4(0, 0, 0, 1) };

    if (tig < MAX_PRIMS)
    {
        triangles[tig] = uint3(0, 1, 2);
        primitives[tig] = { WaveGetWaveIndex(), WaveGetNumWaves() };
    }
}

struct Payload
{
    uint waveIndex;
    uint numWaves;
};

// CHECKAMP-DAG: OpDecorate {{.*}} BuiltIn SubgroupId
// CHECKAMP-DAG: OpDecorate {{.*}} BuiltIn NumSubgroups
[numthreads(1, 1, 1)]
void ampMain()
{
    Payload p;
    p.waveIndex = WaveGetWaveIndex();
    p.numWaves = WaveGetNumWaves();
    DispatchMesh(1, 1, 1, p);
}
