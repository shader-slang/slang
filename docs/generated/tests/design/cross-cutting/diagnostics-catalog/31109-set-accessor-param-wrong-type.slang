//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=f4f5b3d2c9ef9214845827ac5d06dd5ddbba3f3e0cfd7a4c0e837032389e3382
//META: purpose=Fires diagnostic E31109 (set-accessor-param-wrong-type) -- 'set' parameter type mismatch
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=31109
//META: catalog_name=set-accessor-param-wrong-type
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

struct S {
    int x;
    __subscript(int i) -> int {
        get { return x; }
        set(float v) { x = int(v); }
    }
}


//CHECK: E31109
