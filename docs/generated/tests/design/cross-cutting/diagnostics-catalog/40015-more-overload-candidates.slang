//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=ad9264b7212ecc540b0f75f619ef269b65f0e7c3e53e89e7d0f22d84f4c92be5
//META: purpose=Fires diagnostic E40015 (more-overload-candidates) - ~count:Int more overload candidates
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=40015
//META: catalog_name=more-overload-candidates
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// When an overloaded operator has more candidates than the diagnostic prints,
// the remainder are summarized in a trailing note rather than listed. Applying
// unary `*` to an `int` fails against the whole `operator*` overload set, and
// the CHECK pins that summary note's code.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):
void main() { int x = 0; int y = *x; }

//CHECK: E40015
