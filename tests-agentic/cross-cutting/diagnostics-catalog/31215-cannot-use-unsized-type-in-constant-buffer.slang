//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:00:00+00:00
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=bfe3e1b6f6d44fbabde610fbc210a633063a505de3d1bde448fef98cb526fd73
//META: purpose=Fires diagnostic E31215 (cannot-use-unsized-type-in-constant-buffer) -- unsized type in constant buffer
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=31215
//META: catalog_name=cannot-use-unsized-type-in-constant-buffer
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

struct S {
    int data[];
}
ConstantBuffer<S> cb;
[shader("compute")][numthreads(1,1,1)]
void main() {}


// CHECK: E31215
