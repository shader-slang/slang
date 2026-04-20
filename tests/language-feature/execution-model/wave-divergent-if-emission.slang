//TEST:SIMPLE(filecheck=CHECK_HLSL): -target hlsl -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=CHECK_GLSL): -target glsl -stage compute -entry computeMain

RWStructuredBuffer<int> outputBuffer;

[numthreads(4, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    int idx = int(tid.x);
    int branchMin = 0;
    if ((idx & 1) == 0)
        branchMin = WaveActiveMin(idx);
    else
        branchMin = WaveActiveMin(idx);
    int total = WaveActiveSum(branchMin);
    outputBuffer[idx] = total;
}

// CHECK_HLSL: WaveActiveMin
// CHECK_HLSL: WaveActiveMin
// CHECK_HLSL: WaveActiveSum

// CHECK_GLSL: subgroupMin
// CHECK_GLSL: subgroupMin
// CHECK_GLSL: subgroupAdd
