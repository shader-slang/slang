//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#profile-predicates-hlsl-specific
//META: doc_section_digest=3b767d82e05efb8308d4a6e6105f6ce6ede6054c8a92534c2ff2258e80563bdb
//META: purpose=Boundary: `[shader("compute")]` is emitted at the DX shader-model 6.1 profile but not at 6.0 — 6.1 is the documented lower edge of the profile predicate that selects the attribute.
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the exact profile boundary at which the entry-point stage
// attribute starts being emitted. The same kernel is compiled twice with the
// only difference being `cs_6_0` versus `cs_6_1`. CHECK60-NOT asserts the
// attribute is absent one step below the boundary, and CHECK61 asserts it
// appears — spelled with the stage name in quotes — at the boundary itself.
// Both directives also pin `[numthreads(...)]` so a compile that emitted
// nothing at all could not pass by accident.

//TEST:SIMPLE(filecheck=CHECK60):-target hlsl -entry main -stage compute -profile cs_6_0
//TEST:SIMPLE(filecheck=CHECK61):-target hlsl -entry main -stage compute -profile cs_6_1

RWStructuredBuffer<int> outBuf;

[numthreads(4, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = int(tid.x) * 3;
}

// One step below the boundary the stage attribute is not emitted at all.
// CHECK60-NOT: [shader(
// CHECK60: [numthreads(4, 1, 1)]

// At the boundary the stage name is emitted as a quoted string.
// CHECK61: [shader("compute")]
// CHECK61-SAME: [numthreads(4, 1, 1)]
