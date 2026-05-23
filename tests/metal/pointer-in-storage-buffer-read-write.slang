//TEST:SIMPLE(filecheck=CHECK): -target metal -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=METALLIB): -target metallib -stage compute -entry computeMain

// Combined load and store on RWStructuredBuffer<int*>. Both reading a pointer
// element (needs CastIntToPtr after load) and writing a pointer element (needs
// CastPtrToInt before store) must work correctly in the same kernel.
//
// See: https://github.com/shader-slang/slang/issues/10841

RWStructuredBuffer<int*> ptrs;
RWStructuredBuffer<int> output;

// CHECK: ulong device*
// CHECK-NOT: device{{.*}}device

// CHECK-LABEL: computeMain
// CHECK: int device*

// METALLIB: computeMain

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    int* p = ptrs[tid.x];
    output[tid.x] = *p;
    ptrs[tid.x + 1] = p + 1;
}
