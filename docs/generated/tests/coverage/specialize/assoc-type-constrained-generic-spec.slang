//META: generated=true
//META: model=claude-opus-4-8[1m]
//META: generated_at=2026-06-11T14:00:00+00:00
//META: source_commit=ef1068b5485e09b3a7afadba2e25f9541e29af42
//META: doc_ref=docs/generated/design/ir-reference/generics-and-existentials.md#type-flow-specialization
//META: doc_section_digest=50142146a2d3663b95e1c9185f8cc106e919b92f32f63da3bab649a7bb070c03
//META: purpose=A generic constrained by an associated-type equality (where C.Element == int) specializes and calls the conformer's method, returning its stored value.
//META: intent=characterization
//META: covers=source/slang/slang-ir-typeflow-specialize.cpp
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Characterizes the success path where a generic carrying both an interface
// conformance constraint and an associated-type equality constraint is
// statically specialized to a concrete conformer. CHECK asserts the
// interpreter prints 42 — the value stored in the IntBox conformer and read
// back through `c.first()` — which is only reachable if the specialization
// resolved `C = IntBox` and `C.Element = int`.

//TEST:INTERPRET(filecheck=CHECK):

interface IContainer { associatedtype Element; Element first(); }
struct IntBox : IContainer { typedef int Element; int val; int first() { return val; } }

int useContainer<C : IContainer>(C c) where C.Element == int { return c.first(); }

void main()
{
    IntBox b;
    b.val = 42;
    int r = useContainer(b);
    // CHECK: 42
    printf("%d\n", r);
}
