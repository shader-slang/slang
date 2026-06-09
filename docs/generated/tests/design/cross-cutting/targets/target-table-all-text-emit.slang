//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/generated/design/cross-cutting/targets.md#targets
//META: doc_section_digest=e324a10260c794d77c45954ccddaf61cf66af2794e97a8289f9655330ee625d4
//META: purpose=The doc's Targets table lists HLSL/GLSL/SPIR-V/Metal/WGSL/CUDA/C++ as distinct emit backends; the same source compiles successfully to each text-emit target.
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
