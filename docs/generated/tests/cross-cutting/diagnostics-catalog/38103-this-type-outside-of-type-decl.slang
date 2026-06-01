//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:30:00Z
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: purpose=Fires diagnostic E38103 (this-type-outside-of-type-decl) - 'This' used outside aggregate type
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=38103
//META: catalog_name=this-type-outside-of-type-decl
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Using the `This` type from a global function triggers E38103.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

This f()
{
    return 0;
}

// CHECK: E38103
