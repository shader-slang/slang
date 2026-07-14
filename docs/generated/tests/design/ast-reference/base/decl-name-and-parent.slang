//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ast-reference/base.md#decl-declbase
//META: doc_section_digest=e37d0cabbf8f1a75a6de578cd8488f2df3e6f4c6f1f7b7cc4a11429862830842
//META: purpose=Every Decl carries a name and a parent ContainerDecl; the same simple name can live in two different parents and be disambiguated via member-access.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a Decl carries (nameAndLoc, parentDecl). Two structs
// each declare a method `tag()`; member-access `a.tag()` vs `b.tag()`
// resolves through the parent-decl chain. If the parent pointer were
// not threaded, the two same-named decls could not be disambiguated
// and overload resolution at the member-access site would fail.

//TEST:INTERPRET(filecheck=CHECK):

struct A { int tag() { return 7;  } };
struct B { int tag() { return 99; } };

void main()
{
    A a;
    B b;
    //CHECK: a.tag=7
    printf("a.tag=%d\n", a.tag());
    //CHECK: b.tag=99
    printf("b.tag=%d\n", b.tag());
}
