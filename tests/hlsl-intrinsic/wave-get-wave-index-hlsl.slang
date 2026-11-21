//TEST:SIMPLE(filecheck=SM6_0): -target hlsl -entry computeMain -profile cs_6_0
//TEST:SIMPLE(filecheck=SM6_10): -target hlsl -entry computeMain -profile cs_6_10
//TEST:SIMPLE(filecheck=MESH): -target hlsl -entry meshMain -profile ms_6_10
//TEST:SIMPLE(filecheck=AMPLIFICATION): -target hlsl -entry amplificationMain -profile as_6_10

// `WaveGetWaveIndex` and `WaveGetNumWaves` lower to the `GetGroupWaveIndex` and
// `GetGroupWaveCount` intrinsics of Shader Model 6.10. Earlier shader models have no wave index,
// so we compute it from the group index and take the value of the first lane, which tells the
// downstream compiler that it is the same for all lanes of a wave.

RWStructuredBuffer<uint> outputBuffer;

[numthreads(64, 1, 1)]
void computeMain(uint groupIndex : SV_GroupIndex)
{
    outputBuffer[groupIndex] = WaveGetWaveIndex() * 16 + WaveGetNumWaves();
}

struct Vertex
{
    float4 position : SV_Position;
};

[shader("mesh")]
[numthreads(32, 1, 1)]
[outputtopology("triangle")]
void meshMain(out vertices Vertex vertices[3], out indices uint3 triangles[1])
{
    SetMeshOutputCounts(3, 1);
    vertices[WaveGetLaneIndex() % 3].position = float4(WaveGetWaveIndex(), WaveGetNumWaves(), 0, 1);
    triangles[0] = uint3(0, 1, 2);
}

struct Payload
{
    uint waveIndex;
    uint waveCount;
};

[shader("amplification")]
[numthreads(32, 1, 1)]
void amplificationMain()
{
    Payload payload;
    payload.waveIndex = WaveGetWaveIndex();
    payload.waveCount = WaveGetNumWaves();
    DispatchMesh(1, 1, 1, payload);
}

// SM6_0-NOT: GetGroupWave
// SM6_0: WaveReadLaneFirst(
// SM6_0-NOT: GetGroupWave

// SM6_10-NOT: WaveReadLaneFirst
// SM6_10-DAG: GetGroupWaveIndex()
// SM6_10-DAG: GetGroupWaveCount()
// SM6_10-NOT: WaveReadLaneFirst

// MESH-DAG: GetGroupWaveIndex()
// MESH-DAG: GetGroupWaveCount()

// AMPLIFICATION-DAG: GetGroupWaveIndex()
// AMPLIFICATION-DAG: GetGroupWaveCount()
