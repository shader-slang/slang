//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:50:00Z
//META: source_commit=2aa9f69f5e2e75f6e2f4231a451a1a022818e18b
//META: doc_ref=docs/generated/design/target-pipelines/metal.md#phase-a-link-and-entry-point-prep
//META: doc_section_digest=b2ff965a1c32917299c5e0ef14afe738fe74346646f6a636a4ebd2b31af21501
//META: purpose=lowerEnumType collapses an enum to its underlying integer; the emitted Metal text contains the integer literal (int(1)) rather than the enumerator name.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The Phase-A `lowerEnumType` pass erases the enumerator name.
// Reading `Color.Green` (value 1) yields a literal `int(1)` in
// the Metal output; there is no surviving `Green` identifier.

//TEST:SIMPLE(filecheck=CHECK):-target metal -entry main -stage compute

enum Color
{
    Red,
    Green = 1,
    Blue = 2
}

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = int(Color.Green);
}

// CHECK: int(1)
// CHECK-NOT: Green
