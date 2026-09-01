//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=5073a33b5b2656b539f96d27a0aa56e1ba460a3cb01c4121c2320f92ea617847
//META: purpose=Fires diagnostic E30004 (continue-outside-loop) - 'continue' must appear inside loop constructs
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30004
//META: catalog_name=continue-outside-loop
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `continue` needs an enclosing loop to continue. Placed directly in a function
// body it has no target, which the CHECK observes by code.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):
void main() { continue; }

//CHECK: E30004
