//TEST:SIMPLE(filecheck=CHECK): -target metal -stage compute -entry computeMain

// Triple pointer: ensures the fix handles arbitrary nesting depth.
// `device int *device *device *` in a [[buffer(N)]] struct is rejected
// by Metal. The outermost pointer must be lowered to ulong.
//
// See: https://github.com/shader-slang/slang/issues/10841

struct DeepPointer
{
    int*** ptrs;
};

ParameterBlock<DeepPointer> params;

// The struct field type must not contain any pointer chain.
// CHECK: struct DeepPointer
// CHECK: ulong
// CHECK-NOT: device*
// CHECK: };

// The kernel body should cast ulong back and dereference through
// the full pointer chain.
// CHECK: computeMain
// CHECK: int device*

[shader("compute")]
[numthreads(32, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    params.ptrs[0][tid.x][0] = int(tid.x);
}
