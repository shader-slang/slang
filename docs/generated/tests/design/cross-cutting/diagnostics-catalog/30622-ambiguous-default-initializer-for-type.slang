//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=2b0f0927d96d28035499c92ee21219fe722953f68024b3ddf918daba6452d869
//META: purpose=Fires diagnostic E30622 (ambiguous-default-initializer-for-type) -- more than one default initializer was found for type '~type:Type'
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30622
//META: catalog_name=ambiguous-default-initializer-for-type
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

struct S {
    __init() {}
    __init() {}
}
[shader("compute")][numthreads(1,1,1)]
void main() {
    S s;
}


//CHECK: E30622
