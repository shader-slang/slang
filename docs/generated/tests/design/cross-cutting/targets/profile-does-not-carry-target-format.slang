//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/targets.md#profiles
//META: doc_section_digest=e5b9757f49d35cc72a9566431514fc5b19482718219a34a0a4240001ec32891e
//META: purpose=A profile does not carry an output format, so the Shader-Model profile `sm_6_0` pairs with non-D3D targets and each of HLSL, GLSL, SPIR-V, Metal, WGSL, CUDA and C++ still emits its own target-shaped compute kernel.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the format and the profile are independent inputs:
// the profile pins a stage and a version while the output format
// comes from `-target`. One `-profile sm_6_0` request is fanned out
// across every text-emit target; each CHECK asserts that backend's
// own compute marker, so a passing run shows the D3D-family profile
// neither forced HLSL output nor blocked the other backends.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -profile sm_6_0 -entry main -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -profile sm_6_0 -entry main -stage compute
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -profile sm_6_0 -entry main -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -profile sm_6_0 -entry main -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -profile sm_6_0 -entry main -stage compute
//TEST:SIMPLE(filecheck=CUDA):-target cuda -profile sm_6_0 -entry main -stage compute
//TEST:SIMPLE(filecheck=CPP):-target cpp -profile sm_6_0 -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(2, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = int(tid.x);
}

// HLSL: numthreads(2, 1, 1)
// GLSL: layout(local_size_x = 2, local_size_y = 1, local_size_z = 1) in;
// SPIRV: OpExecutionMode %main LocalSize 2 1 1
// Metal fixes the threadgroup size at dispatch rather than in the
// shader, so its compute marker is the `[[kernel]]` qualifier on the
// entry point. `[[` starts a FileCheck variable reference, hence the
// regex block.
// METAL: {{\[\[}}kernel{{\]\]}} void main_{{[0-9]+}}(
// WGSL: @workgroup_size(2, 1, 1)
// CUDA: extern "C" __global__ void main_{{[0-9]+}}()
// CPP: SLANG_PRELUDE_EXPORT
