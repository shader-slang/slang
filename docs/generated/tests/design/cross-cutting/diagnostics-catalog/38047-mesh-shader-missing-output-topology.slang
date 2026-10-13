//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=655566751ab52e5ba00921bed15a7251a9d9e1a8af1f2ee70197b3ee2b7969ac
//META: purpose=Fires diagnostic E38047 (mesh-shader-missing-output-topology) - mesh shader missing [outputtopology] attribute
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=38047
//META: catalog_name=mesh-shader-missing-output-topology
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A mesh shader entry point missing `[outputtopology(...)]`
// triggers E38047.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target spirv -O0

[shader("mesh")]
[numthreads(1,1,1)]
void main() {}

//CHECK: E38047
