//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=2939cdd5ca722bf96e67b41dd7b0f26dee87b43ce66b646b42f9aeb623f22e7a
//META: purpose=Fires diagnostic E29110 (unknown-target-name) - unknown target name
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=29110
//META: catalog_name=unknown-target-name
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `__target_switch` case labels name compilation targets. A label that matches
// no known target is rejected, and the CHECK pins the unknown-target-name code.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target spirv
void main() { __target_switch { case bogus_target_name: break; } }

//CHECK: E29110
