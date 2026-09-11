//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/name-resolution/lookup.md#concepts
//META: doc_section_digest=66115897ba3bf509762cdb486bd39ade9574d8d8ea06eb5213c6d9a683af6562
//META: purpose=The semantic category sits outside the default lookup mask, so a value declared with a system-value semantic's spelling neither hides the semantic annotation nor is hidden by it.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the semantic half of the two mask bits that are outside the
// default mask. The companion attribute test covers the other bit.
// `SV_DispatchThreadID` is declared here as an ordinary constant and
// is also used as the entry point parameter's semantic annotation; the
// two uses ask for different categories, so neither shadows the other.
// The interpreter cannot host an entry point with a system-value
// semantic, so the observation is the emitted HLSL: CHECK asserts the
// parameter still carries the semantic and that the same spelling in
// the expression folded to the user constant's value 11.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -stage compute -entry main

static const int SV_DispatchThreadID = 11;

RWStructuredBuffer<int> outBuf;

[numthreads(1,1,1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    // CHECK: tid_{{[0-9]+}} : SV_DispatchThreadID
    // CHECK: int(11)
    outBuf[0] = int(tid.x) + SV_DispatchThreadID;
}
