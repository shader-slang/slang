//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T22:00:02Z
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#types
//META: doc_section_digest=1b952f18aa4c01c1843e78ee98bfcf640c2d610fbabae24b6d4fd04bdada1ea6
//META: purpose=In HLSL source `signed` is recognized only to diagnose it (E20021) and `unsigned short` is diagnosed as E20022.
//META: intent=negative
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the HLSL-only rejection of `signed` and traditional integer
// names; CHECK-DAG asserts both codes appear.

//TEST:SIMPLE(filecheck=CHECK,exit-code=-1):-target hlsl

void f()
{
    signed int a = 1;
    unsigned short b = 1;
}

// CHECK-DAG: E20021
// CHECK-DAG: E20022
