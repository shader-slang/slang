//TEST:SIMPLE(filecheck=CHECK): -target metal -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=METALLIB): -target metallib -stage compute -entry computeMain

// Transitive struct lowering through various nesting patterns. Metal's
// pointer-to-pointer restriction applies transitively through:
// - Direct struct fields (Inner inner)
// - Multi-dimensional arrays of struct (Inner arr[2][3])
// - Arrays of pointer-to-struct (Inner* ptrArr[2])
//
// See: https://github.com/shader-slang/slang/issues/10841

struct Inner
{
    int** ptr;
};

struct Outer
{
    Inner direct;
    Inner grid[2][3];
    Inner* ptrArr[2];
    int value;
};

ConstantBuffer<Outer> cb;
RWStructuredBuffer<int> output;

// The nested struct's pointer field should be lowered to ulong regardless
// of how it's reached (direct, array-of-array, array-of-pointer).
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
    // Access through direct struct field
    int** pp1 = cb.direct.ptr;

    // Access through multi-dim array of struct
    int** pp2 = cb.grid[0][1].ptr;

    // Access through array of pointer-to-struct
    int** pp3 = cb.ptrArr[0]->ptr;

    output[tid.x] = **pp1 + **pp2 + **pp3 + cb.value;
}
