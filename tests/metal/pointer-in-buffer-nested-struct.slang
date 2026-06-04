//TEST:SIMPLE(filecheck=CHECK): -target metal -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=METALLIB): -target metallib -stage compute -entry computeMain

// Transitive struct lowering through various nesting patterns. Metal's
// pointer-to-pointer restriction applies transitively through:
// - Direct struct fields (Inner inner)
// - Multi-dimensional arrays of struct (Inner arr[2][3])
// - Arrays of pointer-to-struct (Inner* ptrArr[2])
//
// Also exercises passing conventions (in, inout, __ref) on lowered
// structs to verify the framework's cast deferral and function
// specialization handle all parameter-passing modes correctly.
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

int readInner(in Inner i) { return **i.ptr; }
void mutateInner(inout Inner i, int** newPtr) { i.ptr = newPtr; }
int borrowInner(__ref Inner i) { return **i.ptr; }

int readOuter(in Outer o) { return **o.direct.ptr + o.value; }
void mutateOuter(inout Outer o, int v) { o.value = v; }

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

    // Pass Inner by value (in) — receives lowered struct, casts for deref
    int r1 = readInner(cb.direct);

    // Pass Inner by inout — local copy with un-lowered type
    Inner localInner = cb.grid[1][2];
    mutateInner(localInner, cb.direct.ptr);
    int r2 = **localInner.ptr;

    // Pass Inner by __ref — l-value reference to local copy
    Inner refTarget = cb.direct;
    int r3 = borrowInner(refTarget);

    // Pass Outer by value (in) — whole-struct CastIntToPtr reconstruction
    int r4 = readOuter(cb);

    // Pass Outer by inout — local copy, mutated in place
    Outer localOuter = cb;
    mutateOuter(localOuter, 99);
    int r5 = localOuter.value;

    output[tid.x] = **pp1 + **pp2 + **pp3 + r1 + r2 + r3 + r4 + r5;
}
