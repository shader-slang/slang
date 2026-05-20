//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T13:09:57Z
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/llm-generated/ast-reference/declarations.md#aggtypedecl-structdecl-classdecl-enumdecl
//META: doc_section_digest=a5782ff90acc1f9bd796853a7eeead0f64a6539af34d0a231454f705284208e7
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
