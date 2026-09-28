//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=21040668c89a856d31fcfb76b9f2bc203628ea11ca59fed582eb36c5f1bdd90e
//META: purpose=Fires diagnostic E15207 (unknown-language-version) from a minimum-reproduction input.
//META: intent=negative
//META: pipeline_stage=preprocess
//META: catalog_code=15207
//META: catalog_name=unknown-language-version
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Minimum reproduction for diagnostic E15207.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):
#version 99999 hlsl
void main() {}

//CHECK: E15207
