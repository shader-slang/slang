//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:03:13Z
//META: source_commit=ed8f508fc647eecd788a4bd2bb63a4a6f5c80246
//META: doc_ref=docs/generated/design/ast-reference/values.md#witness-and-witness-table-evidence
//META: doc_section_digest=805da103f954782f7e90617d32556351c346f881b305a1705c5b5a6c129787de
//META: purpose=A DeclaredSubtypeWitness represents the proof carried by an InheritanceDecl that T conforms to interface I; the witness is what permits dispatching I's methods on a T value through a generic constraint.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc: "`DeclaredSubtypeWitness` represents the proof carried
// by an `InheritanceDecl` on T". The observable consequence is
// that, given `T : IFoo`, a generic constrained by `T : IFoo`
// can call the interface method on a `T` value through the
// witness.

//TEST:INTERPRET(filecheck=CHECK):

interface IFoo { int foo(); }

struct A : IFoo { int foo() { return 42; } }

int callFoo<T : IFoo>(T v) { return v.foo(); }

void main()
{
    A a;
    // CHECK: r=42
    printf("r=%d\n", callFoo(a));
}
