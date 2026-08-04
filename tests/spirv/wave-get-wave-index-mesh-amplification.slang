// Positive-path test for #11303: WaveGetWaveIndex() / WaveGetNumWaves() are
// permitted on the mesh and amplification (task) execution models in addition
// to compute, matching the compute-class restriction of BuiltIn SubgroupId /
// NumSubgroups (VUID-SubgroupId-SubgroupId-04367 / VUID-NumSubgroups-04293).
//
// This pins the supported stage set so a future change that re-tightens
// `subgroup_workgroup_index` to compute-only would fail CI.

//TEST:SIMPLE(filecheck=MESH): -target spirv-asm -entry meshMain -stage mesh -emit-spirv-directly
//TEST:SIMPLE(filecheck=AMP): -target spirv-asm -entry ampMain -stage amplification -emit-spirv-directly

RWStructuredBuffer<uint> output;

struct Vertex
{
    float4 pos : SV_Position;
}

[shader("mesh")]
[numthreads(1, 1, 1)]
[outputtopology("triangle")]
void meshMain(out vertices Vertex verts[3], out indices uint3 tris[1])
{
    SetMeshOutputCounts(3, 1);
    // MESH: OpLoad {{.*}} %SubgroupId
    output[0] = WaveGetWaveIndex();
    // MESH: OpLoad {{.*}} %NumSubgroups
    output[1] = WaveGetNumWaves();
}

[shader("amplification")]
[numthreads(1, 1, 1)]
void ampMain()
{
    // AMP: OpLoad {{.*}} %SubgroupId
    output[0] = WaveGetWaveIndex();
    // AMP: OpLoad {{.*}} %NumSubgroups
    output[1] = WaveGetNumWaves();
}
