//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/metal.md#phase-a-link-and-entry-point-prep
//META: doc_section_digest=a070776046b78ae88c4449b095d1d0e5fe4aab3b00228002fc1b2b19d4bdde8d
//META: purpose=Boundary: an enumerator whose value is signed `int` MAX (2147483647) collapses through `lowerEnumType` to that decimal literal in the Metal text.
//META: intent=boundary
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Probes `lowerEnumType` at the signed-int MAX edge: the enumerator value
// is the largest representable `int`, so any truncation or re-widening in
// the lowering would change the emitted number. The kernel adds the
// enumerator to a dispatch-derived value (so constant folding cannot drop
// it) and CHECK asserts the exact decimal `2147483647` reaches the MSL
// while the `MaxVal` name does not.

//TEST:SIMPLE(filecheck=CHECK):-target metal -entry main -stage compute

enum E
{
    Zero = 0,
    MaxVal = 2147483647
};

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = int(E.MaxVal) + int(tid.x);
}

// CHECK: 2147483647
// CHECK-NOT: MaxVal
