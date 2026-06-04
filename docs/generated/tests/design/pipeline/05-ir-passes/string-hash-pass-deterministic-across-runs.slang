//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=d245d3be9942f0847c45c138b4c2780e7df19ecb
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#layout-and-binding
//META: doc_section_digest=06fba085c49c1b3a2b0f9fcc4941f5b7e0c382078660bd5b19c43e0aa5b68350
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
