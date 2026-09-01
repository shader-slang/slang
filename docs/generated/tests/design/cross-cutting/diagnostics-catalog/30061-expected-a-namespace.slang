//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=aba2d9513e2e9f3db4635482cd6bbfab1a49423ba19bf8864183224eeabe0997
//META: purpose=Fires diagnostic E30061 (expected-a-namespace) - expected a namespace
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30061
//META: catalog_name=expected-a-namespace
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `using` directive applied to a non-namespace name triggers the diagnostic.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

struct S {}

using S;

void main() {}

//CHECK: E30061
