//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T12:00:00+00:00
//META: source_commit=57fb63455095efe1465661001ad147b24b968932
//META: doc_ref=docs/language-reference/basics-memory-model-special-topics.md#function-parameters-with-inout-inout-modifiers
//META: doc_section_digest=50b3040fe642ee135f57002e37f20cd9b949f33677eec525295c406b3b231822
//META: purpose=`in` is the default parameter direction: GLSL, Metal, CUDA, C++, and WGSL each emit both implicit and explicit `in` parameters as plain value parameters (no pointer or reference indirection).
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a parameter with no direction modifier is treated identically to
// one marked `in` on every text-emit backend other than HLSL (covered by
// in-default-direction-hlsl-emission.slang).  Each directive checks that both
// readImplicit and readExplicit receive plain value parameters.

//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=MTL):-target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CPP):-target cpp -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry computeMain -stage compute

RWStructuredBuffer<int> output;

[noinline]
void readImplicit(int val)
{
    output[2] = val;
}

[noinline]
void readExplicit(in int val)
{
    output[3] = val;
}

[numthreads(1,1,1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    int x = output[0];
    readImplicit(x);
    readExplicit(x);
}

// GLSL: void readImplicit_{{[0-9]+}}(int val_{{[0-9]+}})
// GLSL: void readExplicit_{{[0-9]+}}(int val_{{[0-9]+}})
// GLSL-NOT: inout
// GLSL-NOT: out int

// MTL: void readImplicit_{{[0-9]+}}(int val_{{[0-9]+}}
// MTL: void readExplicit_{{[0-9]+}}(int val_{{[0-9]+}}
// MTL-NOT: int thread*

// CUDA: __device__ void readImplicit_{{[0-9]+}}(int val_{{[0-9]+}})
// CUDA: __device__ void readExplicit_{{[0-9]+}}(int val_{{[0-9]+}})

// CPP: static void readImplicit_{{[0-9]+}}(int32_t val_{{[0-9]+}}
// CPP: static void readExplicit_{{[0-9]+}}(int32_t val_{{[0-9]+}}

// WGSL: fn readImplicit_{{[0-9]+}}( val_{{[0-9]+}} : i32)
// WGSL: fn readExplicit_{{[0-9]+}}( val_{{[0-9]+}} : i32)
// WGSL-NOT: ptr<function
