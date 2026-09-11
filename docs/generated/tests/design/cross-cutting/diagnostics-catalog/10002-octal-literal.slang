//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/compiler-core/slang-lexer-diagnostic-defs.h
//META: doc_section_digest=daf1aec53ab33f862bcaafbe8b3a088da43c615a5ae0b538195fa6134d60888a
//META: purpose=Fires diagnostic E10002 (octal-literal) from a minimum-reproduction input.
//META: intent=negative
//META: pipeline_stage=parse
//META: catalog_code=10002
//META: catalog_name=octalLiteral
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Minimum reproduction for diagnostic E10002.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):
int x = 0123;
void main() { x; }

//CHECK: E10002
