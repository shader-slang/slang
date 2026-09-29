//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=cbc0aebcc119be4945b0b34af434b38faf8e794a8d0c8b3d8206fa64509f8e81
//META: purpose=Fires diagnostic E36105 (unknown-capability) - unknown capability
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=36105
//META: catalog_name=unknown-capability
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A `[require(...)]` listing a name that is not a known
// capability triggers E36105.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

[require(bogus_capability_name)]
void helper() {}

//CHECK: E36105
