//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=c8b5050b6c5a995053c878816dd39e5ffbd69e537c2b8b860166b79a77e883c0
//META: purpose=Fires diagnostic E30071 (var-cannot-be-unsized) - cannot instantiate unsized type
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30071
//META: catalog_name=var-cannot-be-unsized
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A local variable of an unsized array type is rejected.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

void main()
{
    int unsized[];
}

//CHECK: 30071
