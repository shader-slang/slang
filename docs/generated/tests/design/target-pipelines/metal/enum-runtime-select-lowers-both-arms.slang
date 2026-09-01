//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/metal.md#phase-a-link-and-entry-point-prep
//META: doc_section_digest=a070776046b78ae88c4449b095d1d0e5fe4aab3b00228002fc1b2b19d4bdde8d
//META: purpose=An enum value chosen at runtime keeps `lowerEnumType` live through the surviving enum-to-int cast: both branch arms assign bare integer literals to one local.
//META: intent=boundary
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the `reqSet.enumType` gate stays true when the only surviving
// enum construct is a cast: the enum value here is produced by a runtime
// conditional, so constant folding cannot remove the enum type before the
// pass runs. CHECK asserts both branch arms assign plain integer literals
// (`int(2)` / `int(1)`) into a single local, proving the enum was lowered
// to its underlying integer rather than reaching emit as an enum type.

//TEST:SIMPLE(filecheck=CHECK):-target metal -entry main -stage compute

enum Color { Red = 0, Green = 1, Blue = 2 }

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    Color c = (tid.x > 0u) ? Color.Blue : Color.Green;
    outBuf[tid.x] = int(c);
}

// CHECK-DAG: int(2)
// CHECK-DAG: int(1)
// CHECK-NOT: Blue
