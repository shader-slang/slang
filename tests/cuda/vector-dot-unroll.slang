//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry computeMain -stage compute -line-directive-mode none
//TEST:SIMPLE(filecheck=CUDA_NO_LOOP):-target cuda -entry computeMain -stage compute -line-directive-mode none
//TEST:SIMPLE(filecheck=CPP):-target cpp -entry computeMain -stage compute -line-directive-mode none
//TEST:SIMPLE(filecheck=CPP_NO_LOOP):-target cpp -entry computeMain -stage compute -line-directive-mode none

// The generic `dot` fallback in `hlsl.meta.slang` accumulates over a loop, and the C-like
// targets have no native `dot` arm to replace it. The forwarding overload and fallback are
// inlined, while `[ForceUnroll]` turns each specialization into straight-line component
// arithmetic. Without unrolling, the emitted code reaches each component through
// `_slang_vector_get_element` in a runtime loop.
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
    // `vector<T,0>` is a legal width used internally for conditional varying values; it folds to a
    // constant with no loop.
    vector<float, 0> g = (vector<float, 0>)0.0f;
    // The integer `dot` fallback is force-unrolled too, and like float it has no native C-like arm,
    // so it must lower to component arithmetic rather than a `_slang_vector_get_element` loop.
    int3 h = int3((int)inputBuffer[0], (int)inputBuffer[1], (int)inputBuffer[2]);
    int3 k = int3((int)inputBuffer[3], (int)inputBuffer[4], (int)inputBuffer[5]);
    // Keep the specializations in separate assignments. The forwarding overloads are inlined on
    // these targets, so each assignment gives FileCheck a stable boundary for its component count.
    outputBuffer[0] = dot(a, b);
    outputBuffer[1] = dot(c, d);
    outputBuffer[2] = dot(e, f);
    outputBuffer[3] = dot(g, g);
    outputBuffer[4] = (float)dot(h, k);
}

// CUDA-DAG: outputBuffer_{{[0-9]+}})[int(0)]) = {{[^+;]+}} * {{[^+;]+}} + {{[^+;]+}} * {{[^+;]+}};
// CUDA-DAG: outputBuffer_{{[0-9]+}})[int(1)]) = {{[^+;]+}} * {{[^+;]+}} + {{[^+;]+}} * {{[^+;]+}} + {{[^+;]+}} * {{[^+;]+}};
// CUDA-DAG: outputBuffer_{{[0-9]+}})[int(2)]) = {{[^+;]+}} * {{[^+;]+}} + {{[^+;]+}} * {{[^+;]+}} + {{[^+;]+}} * {{[^+;]+}} + {{[^+;]+}} * {{[^+;]+}};

// CPP-DAG: outputBuffer_{{[0-9]+}})[int(0)]) = {{[^+;]+}} * {{[^+;]+}} + {{[^+;]+}} * {{[^+;]+}};
// CPP-DAG: outputBuffer_{{[0-9]+}})[int(1)]) = {{[^+;]+}} * {{[^+;]+}} + {{[^+;]+}} * {{[^+;]+}} + {{[^+;]+}} * {{[^+;]+}};
// CPP-DAG: outputBuffer_{{[0-9]+}})[int(2)]) = {{[^+;]+}} * {{[^+;]+}} + {{[^+;]+}} * {{[^+;]+}} + {{[^+;]+}} * {{[^+;]+}} + {{[^+;]+}} * {{[^+;]+}};

// CUDA-DAG: outputBuffer_{{[0-9]+}})[int(3)]) = 0.0f;
// CPP-DAG: outputBuffer_{{[0-9]+}})[int(3)]) = 0.0f;

// CUDA-DAG: outputBuffer_{{[0-9]+}})[int(4)]) = float({{[^+;]+}} * {{[^+;]+}} + {{[^+;]+}} * {{[^+;]+}} + {{[^+;]+}} * {{[^+;]+}});
// CPP-DAG: outputBuffer_{{[0-9]+}})[int(4)]) = float({{[^+;]+}} * {{[^+;]+}} + {{[^+;]+}} * {{[^+;]+}} + {{[^+;]+}} * {{[^+;]+}});

// CUDA_NO_LOOP-NOT: for(;;)
// CUDA_NO_LOOP-NOT: _slang_vector_get_element
// CUDA_NO_LOOP-NOT: dot_

// CPP_NO_LOOP-NOT: for(;;)
// CPP_NO_LOOP-NOT: _slang_vector_get_element
// CPP_NO_LOOP-NOT: dot_
