//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T15:00:00+00:00
//META: source_commit=ef4af96c0996402dfe65ab0fdd347e4ae7e1a742
//META: doc_ref=docs/llm-generated/ast-reference/base.md#decl-declbase
//META: doc_section_digest=f7adb3aeef4aa0179db4106e8d9801861166f3f45f3b74a6c2615ac6d683e648
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
