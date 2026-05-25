//TEST:SIMPLE(filecheck=CHECK): -target metal -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=METALLIB): -target metallib -stage compute -entry computeMain

// Struct returned by value with a multi-level pointer field. The caller
// extracts the field via FieldExtract (by-value access) rather than
// FieldAddress+Load. The pass must insert CastIntToPtr after the extract.
//
// See: https://github.com/shader-slang/slang/issues/10841

struct Small
{
    int** ptr;
    int val;
};

Small makeSmall(int** p, int v)
{
    Small s;
    s.ptr = p;
    s.val = v;
    return s;
}

ConstantBuffer<Small> cb;
RWStructuredBuffer<int> output;

// The struct should have the pointer field lowered to ulong.
// CHECK-LABEL: struct Small
// CHECK: ulong
// CHECK: };

// CHECK-LABEL: computeMain
// CHECK: int device* device*

// METALLIB: computeMain

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    Small s = makeSmall(cb.ptr, cb.val);
    output[tid.x] = **s.ptr + s.val;
}
