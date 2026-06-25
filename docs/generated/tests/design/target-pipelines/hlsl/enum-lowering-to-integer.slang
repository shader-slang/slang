//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:16:11+00:00
//META: source_commit=ecefa0388fc4ccf7d14670c7bf1eccc88a7bdd14
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#phase-a-link-and-entry-point-prep
//META: doc_section_digest=df36b14c0d0c1c8ecb065a968a44debf9939033194e920e3e5fa7c762e530478
//META: purpose=lowerEnumType collapses an enum to its underlying integer in HLSL emit; the enumerator name does not survive.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Slang's `lowerEnumType` (Phase A, gated on `reqSet.enumType`)
// replaces enum values with their integer payloads before emit;
// the HLSL text contains only the integer literal.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

enum Color { Red, Green, Blue };
RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    Color c = Color.Green;
    if (c == Color.Green)
        outBuf[tid.x] = 1;
    else
        outBuf[tid.x] = 0;
}

// CHECK: outBuf_{{[0-9]+}}[tid_{{[0-9]+}}.x] = int(1)
// CHECK-NOT: Green
// CHECK-NOT: Color
