//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -shaderobj -output-using-type
//TEST:SIMPLE(filecheck=EMIT): -target hlsl -stage compute -entry computeMain
//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)

RWStructuredBuffer<int> outputBuffer;
groupshared int sharedData[4];

[numthreads(4, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID, uint idx : SV_GroupIndex)
{
    sharedData[idx] = int(idx) * 10;
    GroupMemoryBarrierWithGroupSync();
    int neighborIdx = (int(idx) + 1) % 4;
    outputBuffer[idx] = sharedData[neighborIdx];
}

// EMIT: groupshared
// EMIT: GroupMemoryBarrierWithGroupSync

// CHECK: 10
// CHECK: 20
// CHECK: 30
// CHECK: 0
