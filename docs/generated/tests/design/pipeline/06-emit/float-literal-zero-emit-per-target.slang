//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/generated/design/pipeline/06-emit.md#shared-c-like-base
//META: doc_section_digest=9f3194ddc2631c231ae0863cc8c3961c0a5eef6db41560bb17b1114e6c0cf6aa
//META: purpose=Boundary: the float literal 0.0 is emitted as the target's native zero-literal spelling on every C-like text target and as an OpConstant on SPIR-V.
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary probe at the lower edge of the float range: how does each
// backend spell the zero literal? HLSL/CUDA/CPP/WGSL append the `f`
// suffix; GLSL and Metal do not; SPIR-V represents it as an
// OpConstant on the float type.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage compute
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -entry main -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry main -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry main -stage compute
//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry main -stage compute
//TEST:SIMPLE(filecheck=CPP):-target cpp -entry main -stage compute

RWStructuredBuffer<float> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = 0.0;
}

// HLSL: 0.0f
// GLSL: 0.0
// SPIRV: OpConstant %float 0
// METAL: 0.0
// WGSL: 0.0f
// CUDA: 0.0f
// CPP: 0.0f
