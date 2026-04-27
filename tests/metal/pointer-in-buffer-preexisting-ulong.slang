//TEST:SIMPLE(filecheck=CHECK): -target metal -stage compute -entry computeMain

// Verify that a struct with a pre-existing uint64_t (ulong) field alongside
// pointer fields that get lowered to ulong does not cause field conflicts
// or incorrect struct layout. The pre-existing ulong must remain unchanged
// and the lowered pointer must produce a distinct field.
//
// See: https://github.com/shader-slang/slang/issues/10841

struct MyParams
{
    uint64_t addr;
    int** ptrs;
    int* direct;
};

ParameterBlock<MyParams> params;

// Pre-existing uint64_t stays as ulong, int** lowered to ulong,
// int* preserved as typed device pointer. All three should be distinct fields.
// CHECK: struct MyParams
// CHECK: ulong addr
// CHECK: ulong ptrs
// CHECK: int device* direct
// CHECK: };

// CHECK: computeMain

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    int* p = params.ptrs[0];
    p[0] = int(params.addr);
    params.direct[0] = p[0];
}
