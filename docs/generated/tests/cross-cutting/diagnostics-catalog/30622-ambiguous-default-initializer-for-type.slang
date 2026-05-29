//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:00:00+00:00
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=71896719b3573817a4b58ba1eeb0e1fcdb264f9d6e56e11591be299ed37c9225
//META: purpose=Fires diagnostic E30622 (ambiguous-default-initializer-for-type) -- more than one default initializer was found for type '~type:Type'
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30622
//META: catalog_name=ambiguous-default-initializer-for-type
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

struct S {
    __init() {}
    __init() {}
}
[shader("compute")][numthreads(1,1,1)]
void main() {
    S s;
}


// CHECK: E30622
