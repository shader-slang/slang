//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:00:00Z
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: purpose=Fires diagnostic E99996 (note-failed-to-load-dynamic-library) - failed to load dynamic library
//META: intent=negative
//META: pipeline_stage=emit
//META: catalog_code=99996
//META: catalog_name=note-failed-to-load-dynamic-library
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// When the dxil backend tries to load the dxcompiler shared library
// and fails, a 99996 note is emitted alongside the dxil-not-found
// warning.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target dxil -entry main -profile cs_6_0

[numthreads(1,1,1)]
void main() {}

// CHECK: E99996
