//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00Z
//META: source_commit=781ed4aac159d764c5350d082058d68d4f91b785
//META: doc_ref=docs/llm-generated/target-pipelines/metal.md#phase-a-link-and-entry-point-prep
//META: doc_section_digest=b5d7de72928d6c13745ebd9846b3776c3303adefe35d5b4109978f02477d4265
//META: purpose=Boundary: an enum whose enumerator value is signed `int` MAX (2147483647) collapses through `lowerEnumType` to the integer literal in Metal text — the enumerator name (`MaxVal`) does not survive.
//META: intent=boundary
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary axis: enum at the signed-int MAX edge.
// `lowerEnumType` (Phase A) rewrites `E.MaxVal` to `int(2147483647)`
// directly in the emitted Metal text.

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
