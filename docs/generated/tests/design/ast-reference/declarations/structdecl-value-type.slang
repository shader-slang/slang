//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#aggtypedecl-structdecl-classdecl-enumdecl
//META: doc_section_digest=60c3e2924d17c23b1f3bc412881a4c5e71f81efe23ecdd996e436f61429ac8c8
//META: purpose=A StructDecl declares a value-type aggregate; assigning a struct copies it so the original is unchanged.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the documented role of StructDecl as "the value-type
// variant" of AggTypeDecl. Copying a struct into a new local and
// mutating the copy must not affect the original. If StructDecl were
// not value-typed, the original would change.

//TEST:INTERPRET(filecheck=CHECK):

struct Point { int x; int y; }

void main()
{
    Point a;
    a.x = 3; a.y = 4;
    Point b = a;
    b.x = 99;
    //CHECK: a.x=3 b.x=99
    printf("a.x=%d b.x=%d\n", a.x, b.x);
}
