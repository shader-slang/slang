//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:00:00+00:00
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: purpose=Fires diagnostic E30516 (initializer-list-member-visibility-mismatch) -- member '~member:Decl' is ~memberVis:DeclVisibility, but '~type:Type' is ~structVis:DeclVisibility; all members must be ~structVis:DeclVisibility to use an initializer list
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30516
//META: catalog_name=initializer-list-member-visibility-mismatch
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

public struct S {
    private int x;
}
[shader("compute")][numthreads(1,1,1)]
void main() { S s = {5}; }


// CHECK: E30516
