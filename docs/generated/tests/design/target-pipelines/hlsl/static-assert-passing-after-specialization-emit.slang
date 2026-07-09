//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:41:39+00:00
//META: source_commit=199a52d71d360a84e8a13837b913746113a9b510
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#phase-b-specialization-and-type-legalization
//META: doc_section_digest=88de88b16c165f633a8ad6ade9438213eb05e41453c58f2679fe7e345b5df561
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
