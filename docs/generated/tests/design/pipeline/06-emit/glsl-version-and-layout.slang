//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/generated/design/pipeline/06-emit.md#glsl
//META: doc_section_digest=6dca7359790de973663dcbb3b18e060f0e5599c3d24a2921e655fd173f6356a6
//META: purpose=The GLSL backend emits a GLSL source with #version, layout(local_size_x = ...) for the compute stage, and layout(std430, binding = N) for a buffer resource.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The GLSL backend (slang-emit-glsl.{h,cpp}) emits GLSL: a version
// directive at the top, the compute-stage attribute as
// layout(local_size_x = ..., local_size_y = ..., local_size_z = ...),
// and resource bindings as layout(std430, binding = ...).

//TEST:SIMPLE(filecheck=CHECK):-target glsl -entry main -stage compute

[vk::binding(2, 0)]
RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(16, 4, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = int(tid.x);
}

// CHECK: #version
// CHECK: layout(std430, binding = 2)
// CHECK: layout(local_size_x = 16, local_size_y = 4, local_size_z = 1)
// CHECK: void main()
