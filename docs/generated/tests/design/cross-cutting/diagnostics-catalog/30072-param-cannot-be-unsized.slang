//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T11:10:00+00:00
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=0000000000000000000000000000000000000000000000000000000000000000
//META: purpose=Fires diagnostic E30072 (param-cannot-be-unsized) - function parameter cannot be unsized
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30072
//META: catalog_name=param-cannot-be-unsized
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A function parameter of unsized array type is rejected.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

void take(int xs[]) {}

void main() {}

// CHECK: 30072
