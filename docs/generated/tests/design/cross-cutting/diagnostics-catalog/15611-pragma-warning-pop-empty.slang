//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=8c90f1bb715b3ebde63b629306225243b2af01984d38d0771b7f5ea267b5cb7d
//META: purpose=Fires diagnostic E15611 (pragma-warning-pop-empty) - unmatched pop
//META: intent=negative
//META: pipeline_stage=preprocess
//META: catalog_code=15611
//META: catalog_name=pragma-warning-pop-empty
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `#pragma warning(push/pop)` maintains a stack of diagnostic states. A `pop`
// with an empty stack is unmatched, and the CHECK pins that warning code.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):
#pragma warning(pop)
void main() {}

//CHECK: E15611
