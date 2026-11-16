//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/compiler-core/slang-lexer-diagnostic-defs.h
//META: doc_section_digest=6fb9e272b6e8d68e841f64f0078cef85b762ddcd42b466f9c7f1bf05a32fca5d
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
