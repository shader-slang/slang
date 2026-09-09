//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=81cd79e1446d927f13e34e22905167c5d16dd1ace6468b678dad304b5c930694
//META: purpose=Fires diagnostic E32000 (invalid-enum-tag-type) - invalid enum tag type
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=32000
//META: catalog_name=invalid-enum-tag-type
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Using a non-integer type (float) as the enum tag type triggers
// E32000.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

enum E : float
{
    A,
}

//CHECK: E32000
