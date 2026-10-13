//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/compiler-core/slang-lexer-diagnostic-defs.h
//META: doc_section_digest=7030f5f1e47f5f97f674d0d2949bcd3b495e45c10fd0a7eebd6356fd10851faf
//META: purpose=Fires diagnostic E10010 (quote-cannot-be-delimiter) from a minimum-reproduction input.
//META: intent=negative
//META: pipeline_stage=parse
//META: catalog_code=10010
//META: catalog_name=quoteCannotBeDelimiter
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Minimum reproduction for diagnostic E10010.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):
int x = R""(hi)"";
void main() { x; }

//CHECK: E10010
