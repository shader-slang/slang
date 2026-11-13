//TEST:SIMPLE(filecheck=CHECK): -target metal -capability metallib_4_0 -entry computeMain -stage compute

// Metal only accepts integer constants in required_threads_per_threadgroup, so the attribute is omitted.

[vk::constant_id(0)]
const int kSizeX = 8;

RWStructuredBuffer<int> outputBuffer;

// CHECK: function_constant(0)
// CHECK-NOT: required_threads_per_threadgroup
// CHECK: {{\[\[}}kernel{{\]\]}}
[numthreads(kSizeX, 1, 1)]
void computeMain(uint3 dtid: SV_DispatchThreadID)
{
    outputBuffer[dtid.x] = (int)dtid.x;
}
