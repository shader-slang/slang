//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=a7ba21e8237f397b6d686d5b0f121c2267fa38fa83430f29f33d063a55ff5eea
//META: purpose=Fires diagnostic E36106 (expect-capability) - expect a capability name
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=36106
//META: catalog_name=expect-capability
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A `[require(...)]` whose argument is a number rather than a
// capability name triggers E36106.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

[require(42)]
void helper() {}

//CHECK: E36106
