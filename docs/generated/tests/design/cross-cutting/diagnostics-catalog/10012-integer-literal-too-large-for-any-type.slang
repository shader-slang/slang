//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/compiler-core/slang-lexer-diagnostic-defs.h
//META: doc_section_digest=e7a38dd5a2fe772b9212bb11b060b53478bd28ff564f33dadde7d07cd3e4e19e
//META: purpose=Fires diagnostic E10012 (integer-literal-too-large-for-any-type) from a minimum-reproduction input.
//META: intent=negative
//META: pipeline_stage=parse
//META: catalog_code=10012
//META: catalog_name=integerLiteralTooLargeForAnyType
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Minimum reproduction for diagnostic E10012.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):
int x = 99999999999999999999999999;
void main() { x; }

//CHECK: E10012
