//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/cuda.md#phase-d-cuda-emit-and-downstream-tools
//META: doc_section_digest=5fafdc04391fe1499bf74bfe01420955a8364571f1b55e362cc671d3444538c9
//META: purpose=A multi-component swizzle base is never folded into its use sites on CUDA, so the base stays one named temporary and the swizzle is rebuilt element by element as float3 { base.x, base.y, base.z }.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the CUDA emitter, which inherits
// `shouldFoldInstIntoUseSites` from the C++ emitter, refuses to fold a
// value used as the base of a multi-element swizzle. The shader loads a
// float4 from an immutable buffer (so the base is an expensive `__ldg`)
// and writes `v.xyz`. The CHECK captures the temporary name in the
// element-by-element `float3 { ... }` construction and requires the same
// name for all three components: if the base had been folded, each
// component would carry its own copy of the load expression instead.

//TEST:SIMPLE(filecheck=CHECK):-target cuda -entry computeMain -stage compute

StructuredBuffer<float4> src;
RWStructuredBuffer<float3> outBuf;

[shader("compute")]
[numthreads(4, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    float4 v = src[tid.x];
    outBuf[tid.x] = v.xyz;
}

// The load is materialized once into a named temporary ...
// CHECK: = __ldg(
// ... and every swizzle component reads that same temporary.
// CHECK: float3 {[[BASE:[A-Za-z0-9_]+]].x, [[BASE]].y, [[BASE]].z}
