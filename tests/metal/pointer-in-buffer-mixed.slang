//TEST:SIMPLE(filecheck=CHECK): -target metal -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=METALLIB): -target metallib -stage compute -entry computeMain

// Mixed fields: single-level pointer, multi-level pointer, and scalar fields
// in the same ParameterBlock struct. Single-level pointers are preserved as
// typed device pointers; multi-level pointers are lowered to ulong.
//
// See: https://github.com/shader-slang/slang/issues/10841

struct MixedParams
{
    int* data;
    int** indirect;
    float scale;
    int count;
};

ParameterBlock<MixedParams> params;

// Single-level pointer preserved, multi-level lowered to ulong.
// CHECK-LABEL: struct MixedParams
// CHECK: int device* data
// CHECK: ulong indirect
// CHECK: float
// CHECK: int
// CHECK: };

// The kernel body should cast ulong back to a typed pointer for `indirect`.
// CHECK-LABEL: computeMain
// CHECK: int device* device*

// METALLIB: computeMain

[shader("compute")]
[numthreads(32, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    if (tid.x < uint(params.count))
    {
        int* p = params.indirect[0];
        params.data[tid.x] = int(float(p[tid.x]) * params.scale);
    }
}
