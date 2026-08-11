//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=f95dc10b17c6d959f0f721de4f312af290dc6f65134c22f3452778844e58b2c0
//META: purpose=Fires diagnostic E32006 (enum-case-implicit-tag-value-overflow) - implicit enum case value overflows underlying tag type
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=32006
//META: catalog_name=enum-case-implicit-tag-value-overflow
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Setting an explicit case to the max of int and letting the next
// implicit case overflow triggers E32006.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

enum E
{
    A = 2147483647,
    B,
}

//CHECK: E32006