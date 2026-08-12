//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/compiler-core/slang-lexer-diagnostic-defs.h
//META: doc_section_digest=a679fa8a15eccc67618560c02af59347802807fae1d0565096587b5abb89aee7
//META: purpose=Fires diagnostic E10000 (illegal-character-hex) from a minimum-reproduction input.
//META: intent=negative
//META: pipeline_stage=parse
//META: catalog_code=10000
//META: catalog_name=illegalCharacterHex
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Minimum reproduction for diagnostic E10000.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):
int x = 0;
void main() { x; }

//CHECK: E10000
