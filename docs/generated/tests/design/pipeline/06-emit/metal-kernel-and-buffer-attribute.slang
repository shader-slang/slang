//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/06-emit.md#metal
//META: doc_section_digest=2c63512e8c55eaa520cc1fe344ba5b272b854acc441029c722b381d592ccdc2a
//META: purpose=The Metal backend emits Metal Shading Language: the `metal_stdlib` system include, `using namespace metal;`, a `kernel`-qualified entry point, a `thread_position_in_grid` input, and `buffer(N)` argument-table markers.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the Metal backend produces MSL rather than C-like generic text.
// CHECK asserts the Metal-only preamble and the attribute vocabulary Metal
// uses for kernel entry points and buffer arguments. Metal's attributes are
// written with double brackets, which FileCheck reads as a variable
// reference, so the patterns match the bare tokens inside them.

//TEST:SIMPLE(filecheck=METAL):-target metal -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(8, 4, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = int(tid.y);
}

// METAL-DAG: {{^}}#include <metal_stdlib>
// METAL-DAG: {{^}}using namespace metal;
// METAL-DAG: kernel
// METAL-DAG: void main_{{[0-9]+}}(
// METAL-DAG: thread_position_in_grid
// METAL-DAG: buffer(0)
