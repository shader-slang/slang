//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=382e09885d3e96f09a8af119af7bb709aa7e3165f71f86b770256c6a690819b8
//META: purpose=Fires diagnostic E38101 (this-expression-outside-of-type-decl) - 'this' used outside aggregate type
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=38101
//META: catalog_name=this-expression-outside-of-type-decl
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Using `this` from a global function triggers E38101.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

int f()
{
    return this;
}

//CHECK: E38101
