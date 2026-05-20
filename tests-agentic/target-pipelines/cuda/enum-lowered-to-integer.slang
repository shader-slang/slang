//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:30:46+00:00
//META: source_commit=330c9a8d807b9f9352e4754f466d1244ae681cff
//META: doc_ref=docs/llm-generated/target-pipelines/cuda.md#phase-a-link-and-entry-point-prep
//META: doc_section_digest=f2c71c1e818a46614c1369ca340b627ac6a41920fa32fb97d4b7f11c78a611e0
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
