//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=e40615bf80125b7c6c760c63085b29d4b6791ef797e6797ac0f7a473c1fae584
//META: purpose=Fires diagnostic E20001 (unexpected-token-expected-token-type) - unexpected token
//META: intent=negative
//META: pipeline_stage=parse
//META: catalog_code=20001
//META: catalog_name=unexpected-token-expected-token-type
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A stray closing brace after the last declaration has no opening partner. The
// parser reports the unexpected token against the token type it wanted next,
// and the CHECK pins that code.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):
void main() { } }

//CHECK: E20001
