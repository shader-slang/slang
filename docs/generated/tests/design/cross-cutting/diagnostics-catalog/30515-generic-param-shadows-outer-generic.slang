//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:00:00+00:00
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=71ac21893449a3b5abeecac8d93bdd71f8b8bbafd42a97a72e332697af2114ec
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


// CHECK: E30515
