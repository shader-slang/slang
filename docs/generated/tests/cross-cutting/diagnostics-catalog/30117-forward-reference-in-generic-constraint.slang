//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T11:10:00+00:00
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: purpose=Fires diagnostic E30117 (forward-reference-in-generic-constraint) - forward reference in generic constraint
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30117
//META: catalog_name=forward-reference-in-generic-constraint
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Constraint that mentions a generic parameter declared later triggers the diagnostic.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

interface IThing {}

void f<T : U, U : IThing>() {}

void main() {}

// CHECK: 30117
