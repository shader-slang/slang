//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/06-emit.md#backends
//META: doc_section_digest=b39e80b2c14d1286101fea456d02c0ad8020250e1022fdc6eccd5a974bcbef9d
//META: purpose=The same `RWStructuredBuffer<int>` global is rendered in each backend's own resource-binding vocabulary: HLSL `register(u0)`, GLSL `layout(std430, binding = 0)`, SPIR-V `Binding`/`DescriptorSet` decorations, Metal a `buffer(N)` kernel argument, WGSL `@binding(0) @group(0)`, and a `GlobalParams` field on CUDA and C++.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that resource binding is a per-backend emit decision rather than a
// shared spelling. One buffer declared identically in Slang is compiled to
// every text target; each prefix asserts the binding form only that backend
// uses. CUDA and C++ have no binding table at all, so the assertion there is
// that the buffer became a field of the global-parameter struct instead.

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
    outBuf[tid.x] = int(tid.x);
}

// HLSL: register(u0)
// GLSL: layout(std430, binding = 0) buffer
// SPIRV-DAG: OpDecorate %outBuf Binding 0
// SPIRV-DAG: OpDecorate %outBuf DescriptorSet 0
// METAL: buffer(0)
// WGSL: @binding(0) @group(0)
// CUDA: struct GlobalParams_{{[0-9]+}}
// CUDA-NEXT: {
// CUDA-NEXT: RWStructuredBuffer<int> outBuf_{{[0-9]+}};
// CPP: struct GlobalParams_{{[0-9]+}}
// CPP-NEXT: {
// CPP-NEXT: RWStructuredBuffer<int32_t> outBuf_{{[0-9]+}};
