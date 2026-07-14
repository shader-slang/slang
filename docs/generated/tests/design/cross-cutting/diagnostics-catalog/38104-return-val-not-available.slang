//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:30:00Z
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=0000000000000000000000000000000000000000000000000000000000000000
//META: purpose=Fires diagnostic E38104 (return-val-not-available) - '__return_val' not available
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=38104
//META: catalog_name=return-val-not-available
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Using `__return_val` in a function returning a copyable value
// triggers E38104; `__return_val` is only available in functions
// returning a non-copyable value.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

int f()
{
    __return_val = 5;
}

// CHECK: E38104
