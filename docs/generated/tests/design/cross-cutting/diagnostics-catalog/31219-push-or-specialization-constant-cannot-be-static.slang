//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:00:00+00:00
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=4ce24fa35b12f879a2326d5099ddaf798613c25b3c70f9a1c81a264e43a2ab71
//META: purpose=Fires diagnostic E31219 (push-or-specialization-constant-cannot-be-static) -- push/specialization constant storage class error
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=31219
//META: catalog_name=push-or-specialization-constant-cannot-be-static
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

[SpecializationConstant] static const int x = 5;
[shader("compute")][numthreads(1,1,1)]
void main() {}


// CHECK: E31219
