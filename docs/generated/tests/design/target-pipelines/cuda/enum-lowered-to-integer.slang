//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/cuda.md#phase-a-link-and-entry-point-prep
//META: doc_section_digest=f2b0f55be3f7599d8bcbf2797ba30a8b014ef4b949f1acb1eef1c1fcd4a0be49
//META: purpose=lowerEnumType collapses an enum to its underlying integer in CUDA emit; the enumerator name does not appear.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Phase A runs `lowerEnumType` (gated on `reqSet.enumType`). On CUDA
// the result is the integer literal at the use site; the symbol
// `Green` no longer appears in the emitted text.

//TEST:SIMPLE(filecheck=CHECK):-target cuda -entry main -stage compute

enum Color { Red = 7, Green = 12, Blue = 20 }

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    Color c = Color.Green;
    outBuf[tid.x] = (c == Color.Green) ? 1 : 0;
}

// CHECK: int(1)
// CHECK-NOT: Green
