//TEST:SIMPLE(filecheck=GLSL):-target glsl
//TEST:SIMPLE(filecheck=HLSL):-target hlsl

// A `static const` global array whose initializer names a specialization constant is
// valid. Slang treats a specialization constant as a constant expression, not a mutable
// global, so E31226 must not fire for it. Both attribute spellings are covered, alone and
// inside arithmetic. An integer scalar initializer built only from builtin operators folds
// to a specialization-constant value and never reaches the E31226 check. An array
// initializer, or an integer scalar call such as `min(N, 5)`, does not fold, so it reaches
// the check, which must accept it.
//
// The GLSL lines check only the array text. glslang rejects a module-scope `const` array
// over specialization constants, and the direct SPIR-V output for such an array is not
// valid, so this file has no glslang, SPIR-V or Vulkan lane.

// GLSL-NOT: E31226
// HLSL-NOT: E31226

[vk::constant_id(0)]
const int N = 4;

[SpecializationConstant]
const int M = 2;

static const int direct[2] = { N, 3 };
static const int arith[2] = { N + 1, 3 };
static const int repeated[2] = { N, N };
static const int divided[2] = { 64 / N, 32 / N };
static const int otherSpelling[2] = { M, 64 / M };
static const int clamped = min(N, 5);

// GLSL-DAG: direct_0[2] = int[](N_0, 3);
// GLSL-DAG: arith_0[2] = int[](N_0 + 1, 3);
// GLSL-DAG: repeated_0[2] = int[](N_0, N_0);
// GLSL-DAG: divided_0[2] = int[](64 / N_0, 32 / N_0);
// GLSL-DAG: otherSpelling_0[2] = int[](M_0, 64 / M_0);
// GLSL-DAG: min(N_0, 5)

// HLSL-DAG: direct_0[2L] = { globalParams_0.N_0, 3L };
// HLSL-DAG: arith_0[2L] = { globalParams_0.N_0 + 1L, 3L };
// HLSL-DAG: repeated_0[2L] = { globalParams_0.N_0, globalParams_0.N_0 };
// HLSL-DAG: divided_0[2L] = { 64L / globalParams_0.N_0, 32L / globalParams_0.N_0 };
// HLSL-DAG: otherSpelling_0[2L] = { globalParams_0.M_0, 64L / globalParams_0.M_0 };
// HLSL-DAG: min(globalParams_0.N_0, 5L)

RWStructuredBuffer<int> outputBuffer;

[shader("compute")]
[numthreads(2, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint i = tid.x & 1;
    outputBuffer[tid.x] = direct[i] + arith[i] + repeated[i] + divided[i] + otherSpelling[i] + clamped;
}
