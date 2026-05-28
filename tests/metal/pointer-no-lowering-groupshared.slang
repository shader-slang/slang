//TEST:SIMPLE(filecheck=CHECK): -target metal -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=METALLIB): -target metallib -stage compute -entry computeMain

// Verify that a struct with int** fields used in groupshared context is NOT
// lowered, even when the same struct type is also used in a ConstantBuffer
// (which IS lowered). The in-place field mutation for the buffer-bound path
// must not corrupt the groupshared usage.
//
// Metal allows pointer-to-pointer in threadgroup memory — the restriction
// only applies to buffer pointee types.
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

// The groupshared variable should still compile correctly (Metal accepts
// pointer-to-pointer in threadgroup memory).
// CHECK-LABEL: computeMain
// CHECK: threadgroup

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
