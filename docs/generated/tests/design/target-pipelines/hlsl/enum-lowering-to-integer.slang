//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#phase-a-link-and-entry-point-prep
//META: doc_section_digest=d0dd806e881cf8d0bf251730509f805647fcb23a653f4914d05a97f630cbc3c6
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
