//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:30:00Z
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: purpose=Fires diagnostic E39026 (matrix-layout-modifier-on-non-matrix-type) - matrix layout modifier on non-matrix type
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=39026
//META: catalog_name=matrix-layout-modifier-on-non-matrix-type
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Applying `row_major` to a non-matrix (vector) type triggers E39026.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

row_major float v;

// CHECK: E39026
