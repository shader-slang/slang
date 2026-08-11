//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/base.md#decl-declbase
//META: doc_section_digest=e37d0cabbf8f1a75a6de578cd8488f2df3e6f4c6f1f7b7cc4a11429862830842
//META: purpose=Every Decl carries a name and a parent ContainerDecl, so the same simple name declared in two different parents stays distinguishable at a member-access site.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a Decl records both its name and the container it was
// declared in. Two structs each declare a method spelled `tag`; the
// member-access sites `a.tag()` and `b.tag()` must reach different
// declarations. CHECK asserts the two distinct return values, which is
// only possible if each `tag` decl is keyed by its parent as well as
// by its name.

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
