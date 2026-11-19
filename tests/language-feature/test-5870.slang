//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -output-using-type
//TEST:SIMPLE(filecheck=GLSL): -target glsl -entry computeMain
//TEST:SIMPLE(filecheck=SPV): -target spirv-asm -entry computeMain

// Nested GLSL-style array constructors used to be rejected because a
// type-expression bracket chain like `int[2][4]` nested its brackets in
// the opposite order from a declaration `int x[2][4]`. Both spellings
// must denote `Array<Array<int,4>, 2>` (issue #5870).

// Generic counts ride through the type-position re-nesting verbatim:
// `int[NR][NC]` means the same as `int x[NR][NC]`.
int sumGeneric<let NR : int, let NC : int>(int[NR][NC] a)
{
    int s = 0;
    for (int r = 0; r < NR; r++)
        for (int c = 0; c < NC; c++)
            s += a[r][c];
    return s;
}

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4)
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

// A 3-deep asymmetric chain: `L3[1][2][3]` is out of range under any
// non-identity permutation of (2,3,4), so the CHECK pins the exact order.
static const int L3[2][3][4] = int[2][3][4](
    int[3][4](
        int[4](1, 2, 3, 4),
        int[4](5, 6, 7, 8),
        int[4](9, 10, 11, 12)),
    int[3][4](
        int[4](13, 14, 15, 16),
        int[4](17, 18, 19, 20),
        int[4](21, 22, 23, 24)));

// A type spec that is itself a bracket chain combines with declarator brackets
// into one left-to-right chain: `int[3] MIX[2]` is 2 arrays of 3 ints, like
// `int MIX[2][3]`.
static const int[3] MIX[2] = { { 41, 42, 43 }, { 44, 45, 46 } };

// A multi-declarator group with a bracket-chain type spec: each declarator
// folds its own chain, and the shared type spec must stay untouched (the
// group loop restarts each declaration from the SharedTypeExpr wrapper).
static const int[3] GA[2] = { { 51, 52, 53 }, { 54, 55, 56 } },
                  GB[2] = { { 57, 58, 59 }, { 60, 61, 62 } };

// A combined spelling with two declarator brackets splices twice below the
// type spec's own bracket, exercising the splice loop and a two-link re-nest:
// `int[3] MIX3[2][4]` is 2 arrays of 4 arrays of 3 ints, like `int MIX3[2][4][3]`.
static const int[3] MIX3[2][4] = {
    { { 1, 2, 3 }, { 4, 5, 6 }, { 7, 8, 9 }, { 10, 11, 12 } },
    { { 13, 14, 15 }, { 16, 17, 18 }, { 19, 20, 21 }, { 22, 23, 24 } }};

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
    outputBuffer[9] = L3[1][2][3];   // CHECK: 24
    outputBuffer[10] = MIX[1][0];   // CHECK: 44
    outputBuffer[11] = GA[1][0] + GB[1][0]; // CHECK: 114
    outputBuffer[12] = MIX3[1][3][2];       // CHECK: 24
}

// The backend filechecks pin that type-position element addressing folds to
// the same constants the CPU CHECKs see: the addresses B[1][2] and E[1][0]
// evaluate to 7 and 15, which reach codegen with those values (a transposed
// reading would produce different constants or different buffer slots).
//SPV: %int_7 = OpConstant %int 7
//SPV: %int_15 = OpConstant %int 15
//GLSL: _data[uint(2)] = 7;
//GLSL: _data[uint(6)] = 15;
