//TEST:SIMPLE(filecheck=CHECK): -target metal -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=METALLIB): -target metallib -stage compute -entry computeMain

// Verify that a struct with int** fields used in groupshared context is NOT
// lowered, even when the same struct type is also used in a ConstantBuffer
// (which IS lowered).
//
// Mechanism: processModule only discovers types used as buffer element types
// (ConstantBuffer, StructuredBuffer, etc.) and specific pointer address
// spaces (UserPointer, Input, Output) — groupshared variables don't match
// any discovery category. The framework creates a separate lowered type for
// the buffer path, leaving the original (used by groupshared) intact.
//
// The device T* device* restriction only arises from buffer bindings, not
// threadgroup memory — Metal allows pointer-to-pointer in threadgroup.
//
// See: https://github.com/shader-slang/slang/issues/10841

struct SharedData
{
    int** ptr;
    int value;
};

ConstantBuffer<SharedData> cb;
groupshared SharedData sharedCopy;
RWStructuredBuffer<int> output;

// The ConstantBuffer version should have ptr lowered to ulong.
// CHECK-LABEL: struct SharedData
// CHECK: ulong
// CHECK: };

// The original (un-lowered) struct is preserved for the groupshared path.
// Its pointer field must remain as a typed device pointer, not ulong.
// CHECK-LABEL: struct SharedData_0
// CHECK: device* device* ptr
// CHECK: };

// The groupshared variable uses the un-lowered struct type.
// CHECK-LABEL: computeMain
// CHECK: threadgroup SharedData_0

// METALLIB: computeMain

[shader("compute")]
[numthreads(32, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    if (tid.x == 0)
    {
        sharedCopy.ptr = cb.ptr;
        sharedCopy.value = cb.value;
    }
    GroupMemoryBarrierWithGroupSync();
    output[tid.x] = **sharedCopy.ptr + sharedCopy.value;
}
