//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=91245a0d63d2cf068bf89d2ef2188090689c077a31daa62a66c8cabc7fe9b350
//META: purpose=Fires diagnostic E30507 (for-loop-side-effect-changing-different-var) - for loop modifies wrong variable
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30507
//META: catalog_name=for-loop-side-effect-changing-different-var
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A `for` whose increment advances a different variable than its condition tests
// is almost always a copy-paste bug. The checker warns; the CHECK pins the
// side-effect-on-different-variable warning code.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):
void main() { int j = 0; for (int i = 0; i < 4; j++) { } }

//CHECK: E30507
