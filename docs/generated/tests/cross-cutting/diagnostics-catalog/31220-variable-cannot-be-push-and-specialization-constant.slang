//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:00:00+00:00
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: purpose=Fires diagnostic E31220 (variable-cannot-be-push-and-specialization-constant) -- conflicting constant qualifiers
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=31220
//META: catalog_name=variable-cannot-be-push-and-specialization-constant
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

struct C { int x; }
[push_constant][SpecializationConstant] ConstantBuffer<C> pc;
[shader("compute")][numthreads(1,1,1)]
void main() {}


// CHECK: E31220
