//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=ac235039a26eb7486a1fdb21804b6edc5ea11b0eed4428f250e55bd2a10fe9dd
//META: purpose=Fires diagnostic E30100 (static-ref-to-non-static-member) - type cannot refer to non-static member
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30100
//META: catalog_name=static-ref-to-non-static-member
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Referring to a non-static struct member via the type name is invalid.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

struct S { int x; };

void main()
{
    int v = S.x;
}

//CHECK: 30100
