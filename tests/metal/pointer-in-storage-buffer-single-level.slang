//TEST:SIMPLE(filecheck=CHECK): -target metal -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=METALLIB): -target metallib -stage compute -entry computeMain

// Single-level pointer field in RWStructuredBuffer struct. This isolates the
// StorageBuffer rule: the buffer binding itself is device*, so even a
// single-level int* field becomes device int* device* — which Metal rejects.
// The field must be lowered to ulong even though single-level pointers are
// preserved in Uniform/Constant contexts.
//
// See: https://github.com/shader-slang/slang/issues/10841

struct SinglePtrHolder
{
    int* data;
    int value;
};

RWStructuredBuffer<SinglePtrHolder> buf;
RWStructuredBuffer<int> output;

// The single-level pointer field must be lowered to ulong.
// CHECK-LABEL: struct SinglePtrHolder
// CHECK: ulong data
// CHECK: };

// CHECK-LABEL: computeMain
// CHECK: int device*

// METALLIB: computeMain

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    int* p = buf[tid.x].data;
    output[tid.x] = *p + buf[tid.x].value;
}
