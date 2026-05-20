//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/llm-generated/pipeline/06-emit.md#shared-c-like-base
//META: doc_section_digest=9bd773e205e5aa520ee4875c2a6aebc4a9d9aabba8cfac9d3b66489e9488c043
//META: purpose=Boundary: the int32 minimum value (-2147483648) round-trips through emit as a negative literal on each target — HLSL/Metal/CUDA wrap it as `int(-2147483648)`; GLSL emits the bare literal; WGSL casts to i32; SPIR-V encodes it as an OpConstant on %int.
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary probe at the lower edge of the int32 range. Each target
// spells the literal slightly differently, but the underlying value
// must round-trip without overflow.

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
    outBuf[tid.x] = -2147483648;
}

// HLSL: int(-2147483648)
// GLSL: -2147483648
// SPIRV: OpConstant %int -2147483648
// METAL: int(-2147483648)
// WGSL: i32(-2147483648)
// CUDA: int(-2147483648)
// CPP: int(-2147483648)
