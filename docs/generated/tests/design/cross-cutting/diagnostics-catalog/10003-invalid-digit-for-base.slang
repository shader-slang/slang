//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/compiler-core/slang-lexer-diagnostic-defs.h
//META: doc_section_digest=73d602c51478c3e0acf4c38f40070940bb73d2cb46080b2cbb5ded750a173e61
//META: purpose=Fires diagnostic E10003 (invalid-digit-for-base) from a minimum-reproduction input.
//META: intent=negative
//META: pipeline_stage=parse
//META: catalog_code=10003
//META: catalog_name=invalidDigitForBase
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Minimum reproduction for diagnostic E10003.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):
int x = 0b2;
void main() { x; }

//CHECK: E10003
