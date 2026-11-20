//TEST:SIMPLE(filecheck=CHECK,exit-code=2): -target hlsl -entry computeMain -stage compute

import glsl;

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    const matrix<float, 5, 5> m5 = matrix<float, 5, 5>(1.0);
    outputBuffer[0] = int(inverse(m5)[0][0]);
    // CHECK: E41400
    // CHECK: static assertion failed, inverse is only implemented up to 4x4 matrices
}
