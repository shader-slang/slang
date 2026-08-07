//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry computeMain -stage compute -line-directive-mode none
//TEST:SIMPLE(filecheck=CUDA_NO_LOOP):-target cuda -entry computeMain -stage compute -line-directive-mode none
//TEST:SIMPLE(filecheck=CPP):-target cpp -entry computeMain -stage compute -line-directive-mode none
//TEST:SIMPLE(filecheck=CPP_NO_LOOP):-target cpp -entry computeMain -stage compute -line-directive-mode none

// The generic `dot` fallback in `hlsl.meta.slang` accumulates over a loop, and the C-like
// targets have no native `dot` arm to replace it. Without `[ForceUnroll]` the emitted code
// keeps the loop and reaches each component through `_slang_vector_get_element`, so a
// specialized `dot` costs a helper call plus dynamic indexing per element.
//
// The negative assertions live under their own prefixes because a `CHECK-NOT` followed by a
// positive directive only covers the region up to that match; alone in a prefix it covers the
// whole input.
//
// Operands come from a buffer so the assertions are about unrolling rather than operand spelling:
// with literals, a future constant fold could remove the loop for reasons unrelated to
// `[ForceUnroll]` and the negative prefixes would still pass.
//
// The loop-free assertions describe the default and `-O0` levels, which is what runs here.
// `-minimum-slang-optimization` is a separate option, and under it the unrolled body keeps emptied
// `for(;;)` scaffolding that the usual simplification removes — but `_slang_vector_get_element` is
// gone there too, so the dynamic indexing this test exists to catch is fixed at either level.

RWStructuredBuffer<float> inputBuffer;
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    float2 a = float2(inputBuffer[0], inputBuffer[1]);
    float2 b = float2(inputBuffer[2], inputBuffer[3]);
    float3 c = float3(inputBuffer[4], inputBuffer[5], inputBuffer[6]);
    float3 d = float3(inputBuffer[7], inputBuffer[8], inputBuffer[9]);
    float4 e = float4(inputBuffer[10], inputBuffer[11], inputBuffer[12], inputBuffer[13]);
    float4 f = float4(inputBuffer[14], inputBuffer[15], inputBuffer[16], inputBuffer[17]);
    // `vector<T,0>` is a legal width used internally for conditional varying values, so it has to
    // survive the `i < 4` bound as well; it folds to a constant with no loop.
    vector<float, 0> g = (vector<float, 0>)0.0f;
    outputBuffer[0] = dot(a, b) + dot(c, d) + dot(e, f) + dot(g, g);
}

// CUDA-DAG: return x_{{[0-9]+}}.x * y_{{[0-9]+}}.x + x_{{[0-9]+}}.y * y_{{[0-9]+}}.y;
// CUDA-DAG: return x_{{[0-9]+}}.x * y_{{[0-9]+}}.x + x_{{[0-9]+}}.y * y_{{[0-9]+}}.y + x_{{[0-9]+}}.z * y_{{[0-9]+}}.z;
// CUDA-DAG: return x_{{[0-9]+}}.x * y_{{[0-9]+}}.x + x_{{[0-9]+}}.y * y_{{[0-9]+}}.y + x_{{[0-9]+}}.z * y_{{[0-9]+}}.z + x_{{[0-9]+}}.w * y_{{[0-9]+}}.w;

// CPP-DAG: return x_{{[0-9]+}}.x * y_{{[0-9]+}}.x + x_{{[0-9]+}}.y * y_{{[0-9]+}}.y;
// CPP-DAG: return x_{{[0-9]+}}.x * y_{{[0-9]+}}.x + x_{{[0-9]+}}.y * y_{{[0-9]+}}.y + x_{{[0-9]+}}.z * y_{{[0-9]+}}.z;
// CPP-DAG: return x_{{[0-9]+}}.x * y_{{[0-9]+}}.x + x_{{[0-9]+}}.y * y_{{[0-9]+}}.y + x_{{[0-9]+}}.z * y_{{[0-9]+}}.z + x_{{[0-9]+}}.w * y_{{[0-9]+}}.w;

// CUDA-DAG: return 0.0f;

// CUDA_NO_LOOP-NOT: for(;;)
// CUDA_NO_LOOP-NOT: _slang_vector_get_element

// CPP_NO_LOOP-NOT: for(;;)
// CPP_NO_LOOP-NOT: _slang_vector_get_element
