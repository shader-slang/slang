//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#type-defining-declarations
//META: doc_section_digest=fb21c47d3ae888177f02abe81a3052f9206d46318a09404685183a1509335f2b
//META: purpose=`TypedefDecl` parses as `'typedef' Type Declarator ';'` (C-style order: type before the alias declarator).
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the C-style TypedefDecl production with the simplest
// declarator (a bare IDENT): `typedef Type Name;`. The order is
// reversed from `typealias` — type first, alias declarator second —
// and the typedef'd name is usable as a type.

//TEST:INTERPRET(filecheck=CHECK):

typedef int Distance;

void main()
{
    Distance d = 7;
    // CHECK: d=7
    printf("d=%d\n", d);
}
