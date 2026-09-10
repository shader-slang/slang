//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#type-defining-declarations
//META: doc_section_digest=fb21c47d3ae888177f02abe81a3052f9206d46318a09404685183a1509335f2b
//META: purpose=`StructMember` accepts `VarDecl` members produced by `Type IDENT` C-style declarators.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a `StructDecl` body parses two `VarDecl` members in
// C-style spelling (`Type IDENT ';'`). The field values are observed
// after construction to prove the declarators bound names.

//TEST:INTERPRET(filecheck=CHECK):

struct Pair { int a; int b; }

void main()
{
    Pair p;
    p.a = 3;
    p.b = 4;
    // CHECK: 3,4
    printf("%d,%d\n", p.a, p.b);
}
