//TEST:SIMPLE(filecheck=GLSL):-target glsl
//TEST:SIMPLE(filecheck=HLSL):-target hlsl

// A `static const` global array whose initializer names a specialization constant is
// valid. The value of a specialization constant is fixed before the shader runs, so it is
// not a mutable global, and E31226 must not fire for it. Both attribute spellings are
// covered, alone and inside arithmetic. An integer scalar `static const` initializer
// constant-folds and never reaches the E31226 check; an array initializer does not fold,
// so it does.

// GLSL-NOT: E31226
// HLSL-NOT: E31226

[vk::constant_id(0)]
const int N = 4;

[SpecializationConstant]
const int M = 2;

static const int direct[2] = { N, 3 };
static const int arith[2] = { N + 0, 3 };
static const int repeated[2] = { N, N };
static const int divided[2] = { 64 / N, 32 / N };
static const int otherSpelling[2] = { M, 64 / M };

// GLSL-DAG: direct_0[2] = int[](N_0, 3);
// GLSL-DAG: arith_0[2] = int[](N_0, 3);
// GLSL-DAG: repeated_0[2] = int[](N_0, N_0);
// GLSL-DAG: divided_0[2] = int[](64 / N_0, 32 / N_0);
// GLSL-DAG: otherSpelling_0[2] = int[](M_0, 64 / M_0);

// HLSL-DAG: direct_0[int(2)] = { globalParams_0.N_0, int(3) };
// HLSL-DAG: arith_0[int(2)] = { globalParams_0.N_0, int(3) };
// HLSL-DAG: repeated_0[int(2)] = { globalParams_0.N_0, globalParams_0.N_0 };
// HLSL-DAG: divided_0[int(2)] = { int(64) / globalParams_0.N_0, int(32) / globalParams_0.N_0 };
// HLSL-DAG: otherSpelling_0[int(2)] = { globalParams_0.M_0, int(64) / globalParams_0.M_0 };

RWStructuredBuffer<int> outputBuffer;

[shader("compute")]
[numthreads(2, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint i = tid.x & 1;
    outputBuffer[tid.x] = direct[i] + arith[i] + repeated[i] + divided[i] + otherSpelling[i];
}
