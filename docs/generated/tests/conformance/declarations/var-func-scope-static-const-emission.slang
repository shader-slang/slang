//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:58:38+00:00
//META: source_commit=a01e79278cf0be9772deb8b2be52ee87687697a3
//META: doc_ref=docs/language-reference/declarations.md#variables
//META: doc_section_digest=98b79b7c34fc8faf47b9b15881a4299669d58061046761d5503ddf01389e9d47
//META: purpose=A function-scope `static const` variable behaves like a global constant but is visible only within the function; its value folds at compile-time.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a function-scope `static const` folds its compile-time value into every
// text-emit backend. `LOCAL_K * 2.0 = 5.43656...` is stored in an RWStructuredBuffer;
// each emitted-text CHECK asserts the folded literal appears, confirming the constant
// was resolved before target emission.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=SPV):-target spirv-asm -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CPP):-target cpp -entry computeMain -stage compute

RWStructuredBuffer<float> output;
[numthreads(1, 1, 1)]
void computeMain(uint3 id : SV_DispatchThreadID)
{
    static const float LOCAL_K = 2.71828;
    output[0] = LOCAL_K * 2.0;
}

// HLSL: 5.43656
// SPV: OpConstant {{.*}} 5.43656
// GLSL: 5.43656
// METAL: 5.43656
// WGSL: 5.43656
// CUDA: 5.43656
// CPP: 5.43656
