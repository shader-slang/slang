//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=1c7b9ade9ea4b2da14443dc557193ba9e3466c3d62a082928ac0e4a3e11b6b14
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


//CHECK: E30516
