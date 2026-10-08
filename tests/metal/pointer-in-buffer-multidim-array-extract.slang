//TEST:SIMPLE(filecheck=CHECK): -target metal -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=METALLIB): -target metallib -stage compute -entry computeMain

// FieldExtract on a multi-dimensional array of multi-level pointers
// returned by value. The pass must walk nested GetElement chains,
// only casting at the innermost scalar dimension.
//
// See: https://github.com/shader-slang/slang/issues/10841

struct MultiDim
{
    int** ptrs[2][3];
    int tag;
};

MultiDim makeMultiDim(int** p, int t)
{
    MultiDim result;
    result.ptrs[0][0] = p;
    result.tag = t;
    return result;
}

ConstantBuffer<MultiDim> cb;
RWStructuredBuffer<int> output;

// CHECK-LABEL: struct MultiDim
// CHECK: ulong
// CHECK: };

// CHECK-LABEL: computeMain
// CHECK: int device* device*

// METALLIB: computeMain

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    MultiDim md = makeMultiDim(cb.ptrs[0][0], cb.tag);
    output[tid.x] = **md.ptrs[0][0] + md.tag;
}
