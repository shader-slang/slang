//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/values.md#witness-and-witness-table-evidence
//META: doc_section_digest=5988cb257320ec90e43b3610a64f2e3e3057f99ab3613c24fa88fd9583dcee33
//META: purpose=Composing two conformance proofs along an inheritance chain yields the proof for the endpoints, so a type declared to implement a derived interface satisfies a constraint written on the base interface.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the composed form of conformance evidence. `S` declares only
// `S : IDerived` and `IDerived` declares only `IDerived : IBase`, so no
// single declaration states `S : IBase`; the call to callBase(s) can
// only check if the two declared proofs were composed. CHECK asserts
// the base-interface method result, and the second call through the
// derived-constrained generic shows both links are separately usable.

//TEST:INTERPRET(filecheck=CHECK):

interface IBase { int base(); }
interface IDerived : IBase { int derived(); }

struct S : IDerived
{
    int base()    { return 1; }
    int derived() { return 2; }
}

int callBase<T : IBase>(T v) { return v.base(); }
int callDerived<T : IDerived>(T v) { return v.derived(); }

void main()
{
    S s;
    // CHECK: base=1 derived=2
    printf("base=%d derived=%d\n", callBase(s), callDerived(s));
}
