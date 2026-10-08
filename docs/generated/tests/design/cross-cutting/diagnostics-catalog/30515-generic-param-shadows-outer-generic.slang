//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=24dd7590b01410e0cfb1ba8749c90d68ba49f50fb3216daebd8770f27438f7da
//META: purpose=Fires diagnostic E30515 (generic-param-shadows-outer-generic) -- generic parameter '~param:Decl' shadows a generic parameter from an enclosing scope
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30515
//META: catalog_name=generic-param-shadows-outer-generic
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

struct Outer<T> {
    void f<T>() {}
}


//CHECK: E30515
