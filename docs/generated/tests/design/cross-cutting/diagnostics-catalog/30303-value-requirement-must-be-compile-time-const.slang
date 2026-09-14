//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=84a69a8b61ef5b94645c7809b9c872b87ef547c2db2ecb5bd74befd88a18a8ec
//META: purpose=Fires diagnostic E30303 (value-requirement-must-be-compile-time-const) - value requirement requires static const
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30303
//META: catalog_name=value-requirement-must-be-compile-time-const
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// An interface value requirement without `static const` is invalid.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

interface IHasCount
{
    int count;
}

void main() {}

//CHECK: 30303
