//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=dcc4ee96939e7d3a42e8390de86ffd7884152ddbae44d678b0feb8cb8d5f2817
//META: purpose=Fires diagnostic E38007 (entry-point-has-no-stage) - no stage specified for entry point
//META: intent=negative
//META: pipeline_stage=link
//META: catalog_code=38007
//META: catalog_name=entry-point-has-no-stage
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// An entry point's stage comes from `-stage` or from a `[shader(...)]`
// attribute. With neither, the compiler cannot classify the entry point, and
// the CHECK pins the no-stage-specified error.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl -entry main
[numthreads(1,1,1)]
void main() {}

//CHECK: E38007
