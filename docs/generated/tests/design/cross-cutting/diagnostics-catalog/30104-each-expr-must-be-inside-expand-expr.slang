//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=eca4f595a139d8d84a6318960ba367a318cdab23ec9e0bff54dbddeff293592f
//META: purpose=Fires diagnostic E30104 (each-expr-must-be-inside-expand-expr) - 'each' must be inside 'expand'
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30104
//META: catalog_name=each-expr-must-be-inside-expand-expr
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `each` outside an `expand` expression is invalid.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

void f<each T>()
{
    int x = each T;
}

void main() {}

//CHECK: 30104
