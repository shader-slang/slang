//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=335de74375dd7433b49d5463ecfb1f95656f4950fba95402e4b89d9896e96de8
//META: purpose=Fires diagnostic E15001 (directive-without-if) - '~directive' directive without '#if'
//META: intent=negative
//META: pipeline_stage=preprocess
//META: catalog_code=15001
//META: catalog_name=directive-without-if
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `#endif` is only meaningful inside a conditional. A bare `#endif` with no
// matching `#if` is rejected, and the CHECK pins that code.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):
#endif
void main() {}

//CHECK: E15001
