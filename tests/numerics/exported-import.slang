//TEST:SIMPLE: -target hlsl -entry computeMain -stage compute -experimental-feature -I tests/numerics -o exported-import.hlsl

import exported_import_lib;

RWStructuredBuffer<float3> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    float3 result = exportedWave(float3(1.0));
    outputBuffer[0] = result;
}
