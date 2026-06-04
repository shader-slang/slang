//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:30:00Z
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
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

// CHECK: E38048
