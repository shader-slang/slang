//TEST:SIMPLE(filecheck=CHECK): -target metal -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=METALLIB): -target metallib -stage compute -entry computeMain

// FieldExtract on a struct field that is an array of multi-level pointers.
// When a struct with `int** arr[2]` is returned by value (copy, not by
// reference), the FieldExtract instruction produces the full array. The
// pass must set the lowered array type (Array(ulong, 2)) rather than a
// scalar ulong, and defer per-element casts to downstream GEP accesses.
//
// See: https://github.com/shader-slang/slang/issues/10841

struct PtrArray
{
    int** ptrs[2];
    int tag;
};

PtrArray makePtrArray(int** a, int** b, int t)
{
    PtrArray result;
    result.ptrs[0] = a;
    result.ptrs[1] = b;
    result.tag = t;
    return result;
}

ConstantBuffer<PtrArray> cb;
RWStructuredBuffer<int> output;

// The array field should be lowered to array<ulong, 2>.
// CHECK-LABEL: struct PtrArray
// CHECK: ulong
// CHECK-NOT: device{{.*}}device
// CHECK: };

// CHECK-LABEL: computeMain
// CHECK: int device* device*

// METALLIB: computeMain

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    PtrArray pa = makePtrArray(cb.ptrs[0], cb.ptrs[1], cb.tag);
    output[tid.x] = **pa.ptrs[0] + **pa.ptrs[1] + pa.tag;
}
