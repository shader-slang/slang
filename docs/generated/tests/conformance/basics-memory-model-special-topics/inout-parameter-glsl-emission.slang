//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T12:00:00+00:00
//META: source_commit=57fb63455095efe1465661001ad147b24b968932
//META: doc_ref=docs/language-reference/basics-memory-model-special-topics.md#function-parameters-with-inout-inout-modifiers
//META: doc_section_digest=50b3040fe642ee135f57002e37f20cd9b949f33677eec525295c406b3b231822
//META: purpose=`inout` parameter emission on non-HLSL/non-SPIRV backends: GLSL preserves the `inout` keyword; Metal, CUDA, and C++ lower to a pointer parameter; WGSL lowers to `ptr<function, i32>`.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies `inout` parameter lowering on every text-emit backend other than HLSL
// (inout-parameter-hlsl-emission.slang) and SPIR-V (inout-parameter-spirv-emission.slang).
// GLSL keeps `inout`; Metal/CUDA/C++ use a pointer; WGSL uses ptr<function>.

//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=MTL):-target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CPP):-target cpp -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry computeMain -stage compute

RWStructuredBuffer<int> output;

[noinline]
void multiReadWrite(inout int val)
{
    int r1 = val;
    int r2 = val;
    val = r1 + r2;
    int r3 = val;
    val = r3 * 2;
}

[numthreads(1,1,1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    int x = output[0];
    multiReadWrite(x);
    output[0] = x;
}

// GLSL: void multiReadWrite_{{[0-9]+}}(inout int val_{{[0-9]+}})
// GLSL: val_{{[0-9]+}} =

// MTL: void multiReadWrite_{{[0-9]+}}(int thread* val_{{[0-9]+}})
// MTL: *val_{{[0-9]+}} =

// CUDA: __device__ void multiReadWrite_{{[0-9]+}}(int {{.*}}val_{{[0-9]+}})
// CUDA: *val_{{[0-9]+}} =

// CPP: static void multiReadWrite_{{[0-9]+}}(int32_t {{.*}}val_{{[0-9]+}}
// CPP: *val_{{[0-9]+}} =

// WGSL: fn multiReadWrite_{{[0-9]+}}( val_{{[0-9]+}} : ptr<function, i32>)
// WGSL: (*val_{{[0-9]+}}) =
