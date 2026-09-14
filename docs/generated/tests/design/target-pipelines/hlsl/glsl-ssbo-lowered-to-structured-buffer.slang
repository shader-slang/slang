//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#phase-a-link-and-entry-point-prep
//META: doc_section_digest=d0dd806e881cf8d0bf251730509f805647fcb23a653f4914d05a97f630cbc3c6
//META: purpose=HLSL is non-Khronos, so the Phase A gate lets `lowerGLSLShaderStorageBufferObjectsToStructuredBuffers` fire: a GLSL `buffer` block emits as `RWStructuredBuffer<T>` on a `u` register for HLSL, while the Khronos sibling keeps it as a `layout(std430) buffer` block.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the one Phase A pass whose gate the doc singles out as
// firing *because* HLSL is non-Khronos. The claim is a contrast, so
// this is one of the few files in the bundle that compiles the same
// source twice: the HLSL directive asserts the shader-storage block
// has become a native `RWStructuredBuffer` with a register binding
// and that no GLSL `buffer` qualifier survives, and the GLSL
// directive asserts the same declaration stays a `std430 buffer`
// block there — which is only meaningful as evidence that the
// rewrite is target-gated rather than unconditional.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute -allow-glsl
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage compute -allow-glsl

buffer MyBlock
{
    float data[];
} ssbo;

RWStructuredBuffer<float> outBuf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = ssbo.data[tid.x];
}

// HLSL: RWStructuredBuffer<float > ssbo_{{[0-9]+}} : register(u{{[0-9]+}});
// HLSL-NOT: layout(
// HLSL-NOT: {{^}}buffer

// GLSL: layout(std430, binding = {{[0-9]+}}) buffer {{.*}}MyBlock
