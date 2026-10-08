//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=e612d5432e45129c371ebbd0c3515c067de3c64f0436599db7ea1237308abc08
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

//CHECK: E38104
