//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/generated/design/pipeline/06-emit.md#shared-c-like-base
//META: doc_section_digest=9f3194ddc2631c231ae0863cc8c3961c0a5eef6db41560bb17b1114e6c0cf6aa
//META: purpose=Boundary: NaN (0.0/0.0) is emitted as each target's native NaN-producing form — WGSL uses _slang_getNan; HLSL/GLSL/Metal/CUDA/CPP emit the literal division.
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary probe at the special-value edge of the float range: how
// does each backend spell NaN? WGSL synthesises a helper because the
// target language cannot spell `0.0/0.0` as a compile-time literal.

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
    outBuf[tid.x] = 0.0 / 0.0;
}

// HLSL: 0.0f / 0.0f
// GLSL: 0.0 / 0.0
// METAL: 0.0 / 0.0
// WGSL: _slang_getNan
// CUDA: 0.0 / 0.0
// CPP: 0.0 / 0.0
