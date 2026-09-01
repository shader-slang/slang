//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=e40c98bffd30dab6c1db4c5a2bc43d7bf558b03d9e1b6de016141c7ba5a3cf3c
//META: purpose=Fires diagnostic E00016 (unknown-pass-through-target) - unknown pass-through target '~target'
//META: intent=negative
//META: pipeline_stage=parse
//META: catalog_code=16
//META: catalog_name=unknown-pass-through-target
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `-pass-through` names a downstream compiler to hand the source to. An
// unrecognised name is rejected while parsing the command line, which the CHECK
// observes by code.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-pass-through bogusdownstream
[numthreads(1,1,1)]
void main() {}

//CHECK: E00016
