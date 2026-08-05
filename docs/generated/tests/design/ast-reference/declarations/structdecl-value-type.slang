//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#aggtypedecl-structdecl-classdecl-enumdecl
//META: doc_section_digest=ee81272d01f69d54c75b75fc6518d2ad8af9fdb4a6eed0176bdf0fe82ee19e5c
//META: purpose=A StructDecl declares a value-type aggregate; assigning a struct copies it so the original is unchanged.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a struct is the value-type aggregate variant. A struct is
// copied into a second local and the copy is mutated; the CHECK asserts the
// original still holds its old field value, which would not hold if struct
// assignment aliased the same storage.

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
