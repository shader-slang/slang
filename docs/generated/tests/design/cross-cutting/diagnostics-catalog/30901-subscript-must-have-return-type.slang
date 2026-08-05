//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=e2819e444d6e3f6b34734c9f23b4f355edf0380f743dd702ac415ae0a58a3024
//META: purpose=Fires diagnostic E30901 (subscript-must-have-return-type) -- __subscript declaration must have a return type specified after '->'
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30901
//META: catalog_name=subscript-must-have-return-type
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

struct S {
    __subscript(int i) { get { return 1; } }
}


//CHECK: E30901
