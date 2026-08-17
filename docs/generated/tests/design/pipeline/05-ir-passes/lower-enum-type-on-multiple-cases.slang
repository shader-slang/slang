//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#type-and-value-legalization
//META: doc_section_digest=3e3c1fa508130c56014c50d7c74ee56f2e557c277dfbc86c712dbb9373bf794a
//META: purpose=Expansion: lower-enum-type renders all enum cases (four cases here) as plain integer constants in the HLSL emit — the enum type name does not survive in the entry-point body.
//META: intent=expansion
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Variation of the lower-enum claim: a four-case enum (rather than
// two). After lower-enum-type runs each case appears as its integer
// equivalent in the emit and the `Color` spelling is not present in
// the entry-point body.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -stage compute -entry main

uniform int sel;
uniform RWStructuredBuffer<int> buf;

enum Color { Red = 10, Green = 20, Blue = 30, Other = 40 }

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    Color c;
    if (sel == 0) c = Color.Red;
    else if (sel == 1) c = Color.Green;
    else if (sel == 2) c = Color.Blue;
    else c = Color.Other;
    buf[0] = int(c);
}

// HLSL-DAG: 10
// HLSL-DAG: 20
// HLSL-DAG: 30
// HLSL-DAG: 40
