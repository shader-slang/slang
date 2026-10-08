//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=c55b6f6cdf727758c7e077e5a88b4fc9f9e5b1746a0a8fe833f46ecc2082cc2e
//META: purpose=Fires diagnostic E31000 (unknown-attribute-name) -- unknown attribute
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=31000
//META: catalog_name=unknown-attribute-name
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

[unknownAttr]
[shader("compute")][numthreads(1,1,1)]
void main() {}


//CHECK: E31000
