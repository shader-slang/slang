//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=b5eeea1d7fef595114edcd4755a27d174ec47c3287416a9550e2a8eed9c73fe4
//META: purpose=Fires diagnostic E30854 (override-modifier-not-overriding-base-decl) -- '~decl:Decl' marked as 'override' is not overriding any base declarations
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30854
//META: catalog_name=override-modifier-not-overriding-base-decl
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

struct S {
    override int f() { return 1; }
}


//CHECK: E30854
