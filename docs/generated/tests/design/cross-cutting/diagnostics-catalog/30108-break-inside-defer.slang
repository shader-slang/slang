//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=04d58736d8e042362b390c273f7908792b0bd7b75f2e2ee6bed7ac741a3aec60
//META: purpose=Fires diagnostic E30108 (break-inside-defer) - 'break' inside defer
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30108
//META: catalog_name=break-inside-defer
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A `defer` body runs during scope exit, so it may not itself transfer control
// out of that scope. `break` inside `defer` is rejected even though the `defer`
// sits inside a loop, and the CHECK pins that error.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):
void main() { for (int i = 0; i < 2; i++) { defer { break; } } }

//CHECK: E30108
