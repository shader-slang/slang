//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/06-emit.md#glsl
//META: doc_section_digest=822775df5ae4626b6fea7f78ebdd1d058de34fb701c6ac5f2e992147b255eb96
//META: purpose=The GLSL backend emits GLSL source text opening with a `#version` directive, a `layout(local_size_x = ...)` workgroup declaration, and `layout(std430, binding = N)` blocks for buffer resources.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the GLSL backend produces text glslang would accept. A compute
// kernel with one buffer is compiled to GLSL; CHECK asserts the version
// pragma, the std430 buffer block that carries the binding, and the
// `local_size_*` layout qualifier that GLSL uses in place of HLSL's
// `[numthreads]`. The GLSL entry point is always spelled `main`, so the
// dispatch-thread input must come from the `gl_GlobalInvocationID` builtin.

//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(32, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = int(tid.x) - 1;
}

// GLSL: {{^}}#version
// GLSL-DAG: layout(std430, binding = 0) buffer
// GLSL-DAG: layout(local_size_x = 32, local_size_y = 1, local_size_z = 1) in;
// GLSL-DAG: gl_GlobalInvocationID
