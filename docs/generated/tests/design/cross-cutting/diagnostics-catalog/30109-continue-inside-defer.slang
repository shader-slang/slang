//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=ec128d500c7050606b02ba6a8cabd8ece41e2c707e8cf225ec47f7ec3265d2de
//META: purpose=Fires diagnostic E30109 (continue-inside-defer) - 'continue' inside defer
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30109
//META: catalog_name=continue-inside-defer
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `continue` inside a `defer` body would re-enter the loop from a scope that is
// already unwinding. It is rejected with its own code, which the CHECK pins.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):
void main() { for (int i = 0; i < 2; i++) { defer { continue; } } }

//CHECK: E30109
