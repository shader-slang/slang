//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=aedf412c6e1fb09e79b7a097292f021fdccfb3982b1fe84e4187cc9788a05c6b
//META: purpose=Fires diagnostic E31108 (set-accessor-may-not-have-more-than-one-param) -- a 'set' accessor may not have more than one parameter
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=31108
//META: catalog_name=set-accessor-may-not-have-more-than-one-param
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

struct S {
    int x;
    __subscript(int i) -> int {
        get { return x; }
        set(int a, int b) { x = a; }
    }
}


//CHECK: E31108
