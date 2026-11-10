//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/compiler-core/slang-lexer-diagnostic-defs.h
//META: doc_section_digest=8480b913124f9d941c46dd44cdda57f02897044af1afa69345deb97a278cf0ab
//META: purpose=Fires diagnostic E10005 (newline-in-literal) from a minimum-reproduction input.
//META: intent=negative
//META: pipeline_stage=parse
//META: catalog_code=10005
//META: catalog_name=newlineInLiteral
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Minimum reproduction for diagnostic E10005.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):
int x = "hello
world";
void main() { x; }

//CHECK: E10005
