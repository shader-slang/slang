//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/decorations.md#entry-point-and-stage
//META: doc_section_digest=babb40e44ba0eb7addfafeddc37185c8c3bbed4c27b3b7fc8a2469d95b33aca1
//META: purpose=Stress - a compute entry point stacked with numthreads, WaveSize, and earlydepthstencil surfaces all four entry-point/stage decorations on the same function.
//META: intent=stress
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the entry-point/stage decorations are independent and
// co-attachable: one function carries `[numthreads]`, `[WaveSize]` and
// `[earlydepthstencil]` at once. The CHECK-DAG lines assert `entryPoint`,
// `numThreads`, `waveSize` and `earlyDepthStencil` all land on the same
// `func %main`, so a lowering that replaced rather than appended
// decorations would drop one and fail.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<int> buf;

[numthreads(8, 4, 2)]
[WaveSize(32)]
[earlydepthstencil]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = int(tid.x);
}

// CHECK-DAG: [entryPoint({{[0-9]+}} : Int, "main", "{{.*}}")]
// CHECK-DAG: [numThreads(8 : Int, 4 : Int, 2 : Int)]
// CHECK-DAG: [waveSize(32 : Int)]
// CHECK-DAG: [earlyDepthStencil]
// CHECK: func %main
