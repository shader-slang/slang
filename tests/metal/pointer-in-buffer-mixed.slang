//TEST:SIMPLE(filecheck=CHECK): -target metal -stage compute -entry computeMain

// Mixed fields: pointer and non-pointer fields in the same struct.
// Ensures the lowering only converts pointer fields to ulong and
// leaves scalar fields untouched.
//
// See: https://github.com/shader-slang/slang/issues/10841

struct MixedParams
{
    int* data;
    float scale;
    int count;
};

ParameterBlock<MixedParams> params;

// Pointer field becomes ulong; scalar fields remain as-is.
// CHECK: struct MixedParams
// CHECK: ulong
// CHECK: float
// CHECK-NOT: device*
// CHECK: };

// The kernel body should cast ulong back to a typed pointer.
// CHECK: computeMain
// CHECK: (int device*)

[shader("compute")]
[numthreads(32, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    if (tid.x < uint(params.count))
        params.data[tid.x] = int(float(tid.x) * params.scale);
}
