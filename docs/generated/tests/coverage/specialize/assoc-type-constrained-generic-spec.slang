//META: generated=true
//META: model=claude-opus-4-8[1m]
//META: generated_at=2026-06-11T14:00:00+00:00
//META: source_commit=ef1068b5485e09b3a7afadba2e25f9541e29af42
//META: doc_ref=docs/generated/design/ir-reference/generics-and-existentials.md#type-flow-specialization
//META: doc_section_digest=3336b3eca9be44409095a66507ff80c6aa09351d36bfe0e1b016252f5eb2c95d
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
