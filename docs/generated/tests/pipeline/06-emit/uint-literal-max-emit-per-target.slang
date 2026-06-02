//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/generated/design/pipeline/06-emit.md#shared-c-like-base
//META: doc_section_digest=9bd773e205e5aa520ee4875c2a6aebc4a9d9aabba8cfac9d3b66489e9488c043
//META: purpose=Boundary: the uint32 maximum value (4294967295u) round-trips through emit as the target's native unsigned literal — HLSL/GLSL/Metal/CUDA/CPP append a `U` suffix; WGSL wraps it as `u32(...)`; SPIR-V encodes it as an OpConstant on %uint.
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary probe at the upper edge of the uint32 range. The literal
// has no representation as a positive signed int — each backend has
// its own spelling for the unsigned-suffix or unsigned-cast form.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage compute
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -entry main -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry main -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry main -stage compute
//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry main -stage compute
//TEST:SIMPLE(filecheck=CPP):-target cpp -entry main -stage compute

RWStructuredBuffer<uint> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = 4294967295u;
}

// HLSL: 4294967295U
// GLSL: 4294967295U
// SPIRV: OpConstant %uint 4294967295
// METAL: 4294967295U
// WGSL: u32(4294967295)
// CUDA: 4294967295U
// CPP: 4294967295U
