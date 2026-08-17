//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#phase-b-specialization-and-type-legalization
//META: doc_section_digest=bae02bfd9f0c35f37e81a96d88e88829e442c28a56e0f30d0bb769febf165f85
//META: purpose=checkStaticAssert (Phase B direct call) processes static_assert after specialization; a generic-dependent condition satisfied by the concrete argument compiles cleanly and HLSL text is emitted.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the pass/success side of post-specialization static_assert
// processing: `N > 0` is undecidable on the un-specialized generic, so
// reaching HLSL emit at all proves the assertion was evaluated against
// the concrete N = 4 instance. CHECK pins the canonical HLSL prelude
// and entry point to confirm the compile ran to completion.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

RWStructuredBuffer<int> outBuf;

int pick<let N : int>()
{
    static_assert(N > 0, "N must be positive");
    return N * 2;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = pick<4>();
}

// CHECK-DAG: #pragma pack_matrix(column_major)
// CHECK-DAG: RWStructuredBuffer<int{{.*}}> outBuf_{{[0-9]+}}
// CHECK-DAG: void main(
