//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:00:00+00:00
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: purpose=Fires diagnostic E31112 (non-set-accessor-must-not-have-params) -- accessors other than 'set' must not have parameters
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=31112
//META: catalog_name=non-set-accessor-must-not-have-params
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

struct S {
    int x;
    __subscript(int i) -> int {
        get(int extra) { return x; }
    }
}


// CHECK: E31112
