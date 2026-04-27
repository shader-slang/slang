//TEST:SIMPLE(filecheck=CHECK): -target metal -stage compute -entry computeMain

// Standalone RWStructuredBuffer<int*> (not inside a ParameterBlock).
// The late buffer element lowering pass handles this: pointers as data inside
// storage buffers (AddressSpace::StorageBuffer) are always lowered to ulong
// since the buffer itself is already a device pointer.
//
// See: https://github.com/shader-slang/slang/issues/10841

RWStructuredBuffer<int*> ptrs;
RWStructuredBuffer<int> output;

// The storage type should use ulong for the pointer elements.
// CHECK: ulong
// CHECK-NOT: device{{.*}}device

// CHECK: computeMain

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    int* p = ptrs[tid.x];
    output[tid.x] = p[0];
}
