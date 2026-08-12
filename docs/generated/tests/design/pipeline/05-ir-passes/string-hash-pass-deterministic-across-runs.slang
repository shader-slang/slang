//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#layout-and-binding
//META: doc_section_digest=85f9eccdf732defa5a03b9b3664465b1a9ffc9a0344d31f04f8f61bca4e21c71
//META: purpose=Boundary: `getStringHash("")` of the empty string and a non-empty string both reduce to distinct integer literals in HLSL — the string-hash pass folds them deterministically.
//META: intent=boundary
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary input shapes for the string-hash pass: the empty string
// and a single non-empty string. Both are folded to integer
// literals; the string literal text itself does not appear in the
// HLSL emit.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -stage compute -entry main

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = int(getStringHash(""));
    buf[1] = int(getStringHash("hello"));
}

// HLSL: void main
// HLSL-NOT: "hello"
// HLSL-NOT: getStringHash
