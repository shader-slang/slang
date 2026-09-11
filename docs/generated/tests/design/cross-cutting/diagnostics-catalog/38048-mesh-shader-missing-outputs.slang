//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=7da5d191c87dcc93c29341b22b7a87d1f871ed1d2a6fb080429aaf15e1cccf88
//META: purpose=Fires diagnostic E38048 (mesh-shader-missing-outputs) - mesh shader missing OutputVertices/OutputIndices output
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=38048
//META: catalog_name=mesh-shader-missing-outputs
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A mesh shader entry point missing `OutputVertices`/
// `OutputIndices` output parameters triggers E38048.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target spirv -O0

[shader("mesh")]
[outputtopology("triangle")]
[numthreads(1,1,1)]
void main() {}

//CHECK: E38048
