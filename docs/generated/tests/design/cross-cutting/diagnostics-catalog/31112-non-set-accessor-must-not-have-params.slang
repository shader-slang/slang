//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=f348ceff1a856693dc62d8e606e5ab62a28fe675abd49c388240bdabc38b6865
//META: purpose=Fires diagnostic E31112 (non-set-accessor-must-not-have-params) -- accessors other than 'set' must not have parameters
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=31112
//META: catalog_name=non-set-accessor-must-not-have-params
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

struct S {
    int x;
    __subscript(int i) -> int {
        get(int extra) { return x; }
    }
}


//CHECK: E31112
