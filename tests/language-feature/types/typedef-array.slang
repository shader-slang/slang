//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -compute -entry computeMain

// Regression test for shader-slang/slang#11567: a C-style trailing-array typedef
// `typedef <type> <name>[N];` should parse and produce the same array-typed alias
// as the leading-array form `typedef <type>[N] <name>;` (for a single dimension).

// Trailing-array form (the bug): bracket comes AFTER the alias name.
typedef int Arr2[2];

// Leading-array form (already supported) — pinned to confirm 1-D equivalence.
typedef int[3] Arr3;

// Multi-dimensional trailing-array form (non-square, C array-variable semantics:
// `int m[2][3]` is Array<Array<int,3>,2>, so `m[1][2]` is the last element).
typedef int Mat2x3[2][3];

// Routing parseTypeDef through the shared declarator parser also makes the other
// declarator forms a typedef name can carry parse. Declared (unused) only to pin
// that they parse and type-check; the trailing unsized form matches the leading
// `typedef int[] x;` that already compiled.
typedef int Unsized[];
typedef int* IntPtr;

//TEST_INPUT:ubuffer(data=[0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    // Each form writes its own buffer slot so a regression localizes to one form.
    Arr2 a;
    a[0] = 10;
    a[1] = 20;
    // CHECK: 30
    outputBuffer[0] = a[0] + a[1]; // trailing-array alias

    Arr3 b;
    b[0] = 1;
    b[1] = 2;
    b[2] = 3;
    // CHECK: 6
    outputBuffer[1] = b[0] + b[1] + b[2]; // leading-array alias

    Mat2x3 m;
    m[0][0] = 4;
    m[1][2] = 5; // last element of a [2][3] array — out of range if dims were swapped
    // CHECK: 9
    outputBuffer[2] = m[0][0] + m[1][2]; // multi-dim trailing alias
}
