//TEST:SIMPLE(filecheck=CHECK_HLSL): -target hlsl -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=CHECK_GLSL): -target glsl -stage compute -entry computeMain

RWStructuredBuffer<int> outputBuffer;

[numthreads(4, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    int idx = int(tid.x);
    int acc = 0;
    int limit = 5 + (idx & 1);
    for (int i = 0; i < limit; ++i)
    {
        acc += i;
    }
    outputBuffer[idx] = WaveActiveSum(acc);
}

// CHECK_HLSL: for
// CHECK_HLSL: WaveActiveSum

// CHECK_GLSL: for
// CHECK_GLSL: subgroupAdd
