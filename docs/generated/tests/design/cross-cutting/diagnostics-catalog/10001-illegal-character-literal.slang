//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/compiler-core/slang-lexer-diagnostic-defs.h
//META: doc_section_digest=af518dc81beacbbf047dd2bd79f5dfe6a2fa8798385165ce21b183018a6bbf40
//META: purpose=Fires diagnostic E10001 (illegal-character-literal) from a minimum-reproduction input.
//META: intent=negative
//META: pipeline_stage=parse
//META: catalog_code=10001
//META: catalog_name=illegalCharacterLiteral
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Minimum reproduction for diagnostic E10001.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):
int x = '';
void main() { x; }

//CHECK: E10001
