//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/compiler-core/slang-lexer-diagnostic-defs.h
//META: doc_section_digest=85b34b6521ba39bd83e1714446a3200ae29ae30cf258b7799304291426199741
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
