//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=2590d3334d278209ea9fb7c63e6a7b57366d29e530bc5acaf398a0d7e83f93d5
//META: purpose=Fires diagnostic E15612 (pragma-warning-push-not-popped) - unmatched push
//META: intent=negative
//META: pipeline_stage=preprocess
//META: catalog_code=15612
//META: catalog_name=pragma-warning-push-not-popped
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A `#pragma warning(push)` that is never popped leaves the diagnostic-state
// stack unbalanced at end of translation. The CHECK pins the not-popped
// warning code.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):
#pragma warning(push)
void main() {}

//CHECK: E15612
