//TEST:SIMPLE(filecheck=CHECK): -target metal -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=METALLIB): -target metallib -stage compute -entry computeMain

// StructuredBuffer<int*> element load and dereference. The buffer element type
// is lowered to ulong; reading an element must cast back to device int* before
// the dereference operator can be applied.
//
// See: https://github.com/shader-slang/slang/issues/10841

StructuredBuffer<int*> ptrs;
RWStructuredBuffer<int> output;

// CHECK: ulong
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
}
