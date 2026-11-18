//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -output-using-type
//TEST:SIMPLE(filecheck=GLSL): -target glsl -entry computeMain
//TEST:SIMPLE(filecheck=SPV): -target spirv-asm -entry computeMain

// Nested GLSL-style array constructors used to be rejected because a
// type-expression bracket chain like `int[2][4]` nested its brackets in
// the opposite order from a declaration `int x[2][4]`. Both spellings
// must denote `Array<Array<int,4>, 2>` (issue #5870).

// Generic and specialization-constant counts ride through the type-position
// re-nesting verbatim: `int[NR][NC]` means the same as `int x[NR][NC]`.
int sumGeneric<let NR : int, let NC : int>(int[NR][NC] a)
{
    int s = 0;
    for (int r = 0; r < NR; r++)
        for (int c = 0; c < NC; c++)
            s += a[r][c];
    return s;
}

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0 0 0 0 0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

// Both nested-constructor spellings from the issue.
static const int TOP[4] = int[4](0, 0, 1, 0);
static const int BOT[4] = int[4](0, 1, 0, 1);
static const int LUT[2][4] = int[2][4](TOP, BOT);
static const int LUT2[2][4] = int[2][4](int[4](0, 0, 1, 0), int[4](0, 1, 0, 1));

// Type-position brackets must mean the same thing as the equivalent
// declarator spelling: `int[2][4]` is the type of `A` below.
static const int A[2][4] = { { 1, 2, 3, 4 }, { 5, 6, 7, 8 } };
static const int[2][4] B = A;
static const int[2][4] C = int[2][4](A[0], A[1]);

// Single-dimension, unsized, brace-init, and `Array<>` spellings keep working.
static const int D[4] = int[4](9, 10, 11, 12);
static const int G[3] = int[](1, 2, 3);
static const int E[2][2] = { { 13, 14 }, { 15, 16 } };
static const int F[2][4] = Array<Array<int, 4>, 2>(TOP, BOT);

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = LUT[1][3];  // CHECK: 1
    outputBuffer[1] = LUT2[1][2]; // CHECK: 0
    outputBuffer[2] = B[1][2];    // CHECK: 7
    outputBuffer[3] = C[0][3];    // CHECK: 4
    outputBuffer[4] = D[2];       // CHECK: 11
    outputBuffer[5] = G[1];       // CHECK: 2
    outputBuffer[6] = E[1][0];    // CHECK: 15
    outputBuffer[7] = F[0][2];    // CHECK: 1
    outputBuffer[8] = sumGeneric(A); // CHECK: 36
}

// The nested constructors must survive codegen with the right values:
// 7 and 15 only reach the output through B and E.
//SPV: %int_7 = OpConstant %int 7
//SPV: %int_15 = OpConstant %int 15
//GLSL: _data[uint(2)] = 7;
//GLSL: _data[uint(6)] = 15;
