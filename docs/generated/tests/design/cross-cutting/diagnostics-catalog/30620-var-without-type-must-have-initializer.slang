//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=39f348a98921bf590098a1f13d21b85bb41ed7f8b9d0107e1b28733f27af3be7
//META: purpose=Fires diagnostic E30620 (var-without-type-must-have-initializer) -- a variable declaration without an initial-value expression must be given an explicit type
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30620
//META: catalog_name=var-without-type-must-have-initializer
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

var x;
[shader("compute")][numthreads(1,1,1)]
void main() {}


//CHECK: E30620
