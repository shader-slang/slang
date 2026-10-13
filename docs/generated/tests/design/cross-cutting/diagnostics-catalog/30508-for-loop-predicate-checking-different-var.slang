//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=b41d76a35beb92618a27992717892683b73519d52980ec5533c9a7dbd07cdbb0
//META: purpose=Fires diagnostic E30508 (for-loop-predicate-checking-different-var) - for loop predicate checks wrong variable
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30508
//META: catalog_name=for-loop-predicate-checking-different-var
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The mirror-image mistake: the condition tests a variable the increment never
// touches. The CHECK pins the predicate-checks-different-variable warning code.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):
void main() { int j = 0; for (int i = 0; j < 4; i++) { } }

//CHECK: E30508
