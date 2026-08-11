//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/values.md#witness-and-witness-table-evidence
//META: doc_section_digest=5988cb257320ec90e43b3610a64f2e3e3057f99ab3613c24fa88fd9583dcee33
//META: purpose=Proving T conforms to an interface constructs the conformance evidence an inheritance declaration carries, and that evidence is what permits dispatching the interface's methods on a T value inside a generic.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the consumption side of declared conformance evidence: the
// generic body calls `v.foo()` knowing only `T : IFoo`, so the call can
// resolve only through the evidence the conformer's inheritance clause
// produced. Two conformers are passed through the same generic; CHECK
// asserts each returns its own implementation's value, which shows the
// dispatch followed each type's own evidence.

//TEST:INTERPRET(filecheck=CHECK):

interface IFoo { int foo(); }

struct A : IFoo { int foo() { return 42; } }
struct B : IFoo { int foo() { return 43; } }

int callFoo<T : IFoo>(T v) { return v.foo(); }

void main()
{
    A a;
    B b;
    // CHECK: r=42,43
    printf("r=%d,%d\n", callFoo(a), callFoo(b));
}
