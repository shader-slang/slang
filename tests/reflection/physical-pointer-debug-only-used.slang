// https://github.com/shader-slang/slang/issues/13632
//
// With `-g`, a `T*` value that only debug information observes is removed on HLSL, so the buffer
// it was cast from is unused in the reflection, as it is without `-g`.

//TEST:REFLECTION(filecheck=CHECK):-stage compute -entry computeMain -target hlsl -g
//TEST:REFLECTION(filecheck=CHECK):-stage compute -entry computeMain -target hlsl

// CHECK: "name": "addrs",
// CHECK: "used": 0
// CHECK: "name": "outBuf",
// CHECK: "used": 1

RWStructuredBuffer<uint2> addrs;
RWStructuredBuffer<float> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    float* unused = (float*)uint64_t(addrs[tid.x].x);
    outBuf[tid.x] = 1.0;
}
