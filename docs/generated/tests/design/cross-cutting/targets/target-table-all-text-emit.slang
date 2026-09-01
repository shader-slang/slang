//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/targets.md#targets
//META: doc_section_digest=09ecdf153c06b46e85b5bfd65a0136d6c1516c1e42e5e33b4b02ee440f2f963c
//META: purpose=HLSL, GLSL, SPIR-V, Metal, WGSL, CUDA and C++ are distinct emit backends of the target table, and one source compiles successfully to each of those text-emit targets.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's Targets table groups public SlangCompileTarget values by
// emit backend. The text-emit subset of those groups is what this
// no-GPU runner can observe: HLSL, GLSL, SPIR-V asm, Metal, WGSL,
// CUDA, C++. Each //TEST directive below FileChecks a marker that
// only the matching backend would emit. Together they assert the
// targets table is realized in the compiler's emit dispatch.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage compute
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -entry main -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry main -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry main -stage compute
//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry main -stage compute
//TEST:SIMPLE(filecheck=CPP):-target cpp -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = 1;
}

// HLSL: numthreads
// GLSL: #version
// SPIRV: OpEntryPoint GLCompute
// METAL: metal_stdlib
// WGSL: @compute
// CUDA: __global__
// CPP: SLANG_PRELUDE_EXPORT
