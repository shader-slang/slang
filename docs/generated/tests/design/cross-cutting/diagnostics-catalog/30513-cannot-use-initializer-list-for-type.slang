//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:00:00+00:00
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=6b1c030f2a75ad3011484d1e2a0686405f9964830e53bc2dfd301aa524a0b8db
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


// CHECK: E30513
