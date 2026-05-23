//TEST:SIMPLE(filecheck=CHECK): -target metal -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=METALLIB): -target metallib -stage compute -entry computeMain

// Multi-level pointer field inside a nested struct. Metal's pointer-to-pointer
// restriction is transitive: if Inner contains int**, then Outer (which contains
// Inner) cannot be used as a buffer pointee type either. The pass must
// recursively walk nested struct fields to find and lower these.
//
// See: https://github.com/shader-slang/slang/issues/10841

struct Inner
{
    int** ptr;
};

struct Outer
{
    Inner inner;
    int value;
};

ConstantBuffer<Outer> cb;
RWStructuredBuffer<int> output;

// The nested struct's pointer field should be lowered to ulong.
// CHECK-LABEL: struct Inner
// CHECK: ulong
// CHECK: };

// CHECK-LABEL: computeMain
// CHECK: int device* device*

// METALLIB: computeMain

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    int** pp = cb.inner.ptr;
    int* p = *pp;
    output[tid.x] = *p + cb.value;
}
