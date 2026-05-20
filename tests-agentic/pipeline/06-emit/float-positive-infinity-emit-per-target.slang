//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/llm-generated/pipeline/06-emit.md#shared-c-like-base
//META: doc_section_digest=9bd773e205e5aa520ee4875c2a6aebc4a9d9aabba8cfac9d3b66489e9488c043
//META: purpose=Boundary: +infinity (1.0/0.0) is emitted as each target's native infinity spelling — CUDA uses SLANG_INFINITY, WGSL uses _slang_getInfinity, HLSL/GLSL/Metal/CPP emit the literal division.
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary probe at the upper non-finite edge of the float range:
// how does each backend spell +inf? CUDA and WGSL synthesise a helper
// (SLANG_INFINITY / _slang_getInfinity) because their target language
// cannot spell `1.0/0.0` as a compile-time literal; HLSL/GLSL/Metal
// and the C++ backend just emit the division.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry main -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry main -stage compute
//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry main -stage compute
//TEST:SIMPLE(filecheck=CPP):-target cpp -entry main -stage compute

RWStructuredBuffer<float> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = 1.0 / 0.0;
}

// HLSL: 1.0f / 0.0f
// GLSL: 1.0 / 0.0
// METAL: 1.0 / 0.0
// WGSL: _slang_getInfinity
// CUDA: SLANG_INFINITY
// CPP: SLANG_INFINITY
