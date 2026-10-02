// Regression for shader-slang/slang#13376: an array of matrices whose length is a specialization
// constant converts to an array of another layout in a loop, both in a function and in the
// initializer of a global variable.

//TEST:SIMPLE(filecheck=SPV-SAME):-target spirv -entry main -stage compute -matrix-layout-row-major
//TEST:SIMPLE(filecheck=SPV-OTHER):-target spirv -entry main -stage compute -matrix-layout-column-major
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute -matrix-layout-row-major
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute -matrix-layout-column-major

[vk::constant_id(0)]
const int N = 3;

RWStructuredBuffer<float> output;

static row_major float4x4 rowMajorGlobal[N];
static float4x4 converted[N] = rowMajorGlobal;

float last(float4x4 x[N]) { return x[N - 1][0][0]; }

[numthreads(1, 1, 1)]
void main()
{
    row_major float4x4 a[N];
    for (int i = 0; i < N; i++)
        a[i] = float4x4(i);
    output[0] = last(a) + converted[0][0][0];
}

// With a row-major default both conversions are between equal layouts and disappear, leaving only
// the loop in `main`. Otherwise each conversion is a loop of its own.
// SPV-SAME: OpSpecConstant
// SPV-SAME-COUNT-1: OpLoopMerge
// SPV-SAME-NOT: OpLoopMerge
// SPV-OTHER: OpSpecConstant
// SPV-OTHER-COUNT-3: OpLoopMerge
// SPV-OTHER-NOT: OpLoopMerge

// HLSL: void main()
// HLSL: last_0(
