//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:00:00+00:00
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: purpose=Fires diagnostic E31108 (set-accessor-may-not-have-more-than-one-param) -- a 'set' accessor may not have more than one parameter
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=31108
//META: catalog_name=set-accessor-may-not-have-more-than-one-param
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

struct S {
    int x;
    __subscript(int i) -> int {
        get { return x; }
        set(int a, int b) { x = a; }
    }
}


// CHECK: E31108
