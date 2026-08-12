//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=2ae94d29db43e367a56c96c0e9f3e663ef50f4256c963ccca40a4371d7153973
//META: purpose=Fires diagnostic E30513 (cannot-use-initializer-list-for-type) -- cannot use initializer list for type '~type:Type'
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30513
//META: catalog_name=cannot-use-initializer-list-for-type
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

[shader("compute")][numthreads(1,1,1)]
void main() {
    Texture2D t = {1};
}


//CHECK: E30513
