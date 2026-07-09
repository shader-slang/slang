//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T12:00:00+00:00
//META: source_commit=57fb63455095efe1465661001ad147b24b968932
//META: doc_ref=docs/language-reference/basics-memory-model-special-topics.md#function-parameters-with-inout-inout-modifiers
//META: doc_section_digest=50b3040fe642ee135f57002e37f20cd9b949f33677eec525295c406b3b231822
//META: purpose=`in` parameter may be read any number of times: GLSL, Metal, CUDA, C++, and WGSL each emit a plain value parameter that appears multiple times in an addition expression.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that an `in` parameter may be read multiple times on every text-emit
// backend other than HLSL (covered by in-parameter-multiple-reads-hlsl-emission.slang).
// `sumOfReads` reads `val` three times; each directive checks a plain value
// parameter signature and multiple uses of the parameter in the body.

//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=MTL):-target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CPP):-target cpp -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry computeMain -stage compute

RWStructuredBuffer<int> output;

[noinline]
void sumOfReads(in int val)
{
    // read `val` three times
    int r = val + val + val;
    output[0] = r;
}

[numthreads(1,1,1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    int x = output[1];
    sumOfReads(x);
}

// GLSL: void sumOfReads_{{[0-9]+}}(int val_{{[0-9]+}})
// GLSL: val_{{[0-9]+}} + val_{{[0-9]+}} + val_{{[0-9]+}}

// MTL: void sumOfReads_{{[0-9]+}}(int val_{{[0-9]+}}
// MTL: val_{{[0-9]+}} + val_{{[0-9]+}} + val_{{[0-9]+}}

// CUDA: __device__ void sumOfReads_{{[0-9]+}}(int val_{{[0-9]+}})
// CUDA: val_{{[0-9]+}} + val_{{[0-9]+}} + val_{{[0-9]+}}

// CPP: static void sumOfReads_{{[0-9]+}}(int32_t val_{{[0-9]+}}
// CPP: val_{{[0-9]+}} + val_{{[0-9]+}} + val_{{[0-9]+}}

// WGSL: fn sumOfReads_{{[0-9]+}}( val_{{[0-9]+}} : i32)
// WGSL: val_{{[0-9]+}} + val_{{[0-9]+}} + val_{{[0-9]+}}
