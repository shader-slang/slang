//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#memberexpr--staticmemberexpr--derefmemberexpr
//META: doc_section_digest=2e4c0cf09ff9c6fb2f3bd8e1c643d300367d45168a8b35d087ed154695482428
//META: purpose=Stresses MemberExpr chaining: a five-deep `a.b.c.d.e.v` field access must resolve and read the written value.
//META: intent=stress
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// MemberExpr chains build a tree where each `.b` is a MemberExpr
// whose base is the previous MemberExpr. Five nested struct levels
// produce a chain of five MemberExpr nodes; the checker must resolve
// each level and the resulting load must yield the assigned value.

//TEST:INTERPRET(filecheck=CHECK):

struct E { int v; }
struct D { E e; }
struct C { D d; }
struct B { C c; }
struct A { B b; }

void main()
{
    A a;
    a.b.c.d.e.v = 42;
    // CHECK: v=42
    printf("v=%d\n", a.b.c.d.e.v);
}
