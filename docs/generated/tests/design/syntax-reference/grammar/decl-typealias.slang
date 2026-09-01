//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#type-defining-declarations
//META: doc_section_digest=fb21c47d3ae888177f02abe81a3052f9206d46318a09404685183a1509335f2b
//META: purpose=`TypeAliasDecl` parses as `'typealias' IDENT '=' Type ';'`.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the TypeAliasDecl production: `typealias Name = Type;`. The
// alias is then used as a type in a variable declaration.

//TEST:INTERPRET(filecheck=CHECK):

typealias MyInt = int;

void main()
{
    MyInt x = 42;
    // CHECK: x=42
    printf("x=%d\n", x);
}
