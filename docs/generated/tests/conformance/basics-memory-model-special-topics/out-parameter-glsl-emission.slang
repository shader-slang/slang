//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T12:00:00+00:00
//META: source_commit=57fb63455095efe1465661001ad147b24b968932
//META: doc_ref=docs/language-reference/basics-memory-model-special-topics.md#function-parameters-with-inout-inout-modifiers
//META: doc_section_digest=50b3040fe642ee135f57002e37f20cd9b949f33677eec525295c406b3b231822
//META: purpose=`out` parameter emission on non-HLSL/non-SPIRV backends: GLSL preserves the `out` keyword; Metal, CUDA, and C++ lower to a pointer parameter; WGSL lowers to `ptr<function, i32>`.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies `out` parameter lowering on every text-emit backend other than HLSL
// (out-parameter-hlsl-emission.slang) and SPIR-V (out-parameter-spirv-emission.slang).
// GLSL keeps `out`; Metal/CUDA/C++ use a pointer; WGSL uses ptr<function>.

//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=MTL):-target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CPP):-target cpp -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry computeMain -stage compute

RWStructuredBuffer<int> output;

[noinline]
void writeOut(out int result, int factor)
{
    result = factor * 2;
}

[numthreads(1,1,1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    int r = 0;
    int f = output[0];
    writeOut(r, f);
    output[0] = r;
}

// GLSL: void writeOut_{{[0-9]+}}(out int result_{{[0-9]+}},
// GLSL: result_{{[0-9]+}} =

// MTL: void writeOut_{{[0-9]+}}(int thread* result_{{[0-9]+}},
// MTL: *result_{{[0-9]+}} =

// CUDA: __device__ void writeOut_{{[0-9]+}}(int {{.*}}result_{{[0-9]+}},
// CUDA: *result_{{[0-9]+}} =

// CPP: static void writeOut_{{[0-9]+}}(int32_t {{.*}}result_{{[0-9]+}},
// CPP: *result_{{[0-9]+}} =

// WGSL: fn writeOut_{{[0-9]+}}( result_{{[0-9]+}} : ptr<function, i32>
// WGSL: (*result_{{[0-9]+}}) =
