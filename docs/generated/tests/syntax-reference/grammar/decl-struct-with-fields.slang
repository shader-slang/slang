//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:03:32+00:00
//META: source_commit=ed8f508fc647eecd788a4bd2bb63a4a6f5c80246
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#type-defining-declarations
//META: doc_section_digest=1da21f4604b0cc983d0ee89029633af70ee59186fe9ef2a8dafdd1550c122368
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
