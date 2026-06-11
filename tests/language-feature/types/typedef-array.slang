//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -compute -entry computeMain

// Regression test for shader-slang/slang#11567: a C-style trailing-array typedef
// `typedef <type> <name>[N];` should parse and produce the same array-typed alias
// as the leading-array form `typedef <type>[N] <name>;` that already worked.

// Trailing-array form (the bug): bracket comes AFTER the alias name.
typedef int Arr2[2];

// Leading-array form (already supported) — pinned to confirm identical handling.
typedef int[3] Arr3;

// Multi-dimensional trailing-array form.
typedef int Mat2x2[2][2];

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    Arr2 a;
    a[0] = 10;
    a[1] = 20;

    Arr3 b;
    b[0] = 1;
    b[1] = 2;
    b[2] = 3;

    Mat2x2 m;
    m[0][0] = 4;
    m[1][1] = 5;

    // CHECK: 1
    outputBuffer[0] = true && (a[0] + a[1] == 30) // trailing-array alias
                      && (b[0] + b[1] + b[2] == 6) // leading-array alias
                      && (m[0][0] + m[1][1] == 9); // multi-dim trailing alias
}
