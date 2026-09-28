//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=79eb3bee3d2bb43dbf87d54054689bf5fa4ed808d74d8ee0ed246546ed3e8f43
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


//CHECK: E31220
