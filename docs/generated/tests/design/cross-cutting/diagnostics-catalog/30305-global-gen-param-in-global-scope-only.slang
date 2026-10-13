//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=3a6718cdb0733f9d014156d0eaae9a9fbb633007e93fcfc90ba2a6bf226447b1
//META: purpose=Fires diagnostic E30305 (global-gen-param-in-global-scope-only) - type_param outside global scope
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30305
//META: catalog_name=global-gen-param-in-global-scope-only
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A `type_param` declaration inside a function body is invalid.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

void main()
{
    type_param T;
}

//CHECK: 30305
