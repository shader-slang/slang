//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:03:13Z
//META: source_commit=ed8f508fc647eecd788a4bd2bb63a4a6f5c80246
//META: doc_ref=docs/generated/design/ast-reference/values.md#witness-and-witness-table-evidence
//META: doc_section_digest=805da103f954782f7e90617d32556351c346f881b305a1705c5b5a6c129787de
//META: purpose=A TransitiveSubtypeWitness composes two existing witnesses A:B and B:C to obtain A:C; a concrete type that implements a derived interface satisfies a generic constraint on the base interface.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc: "`TransitiveSubtypeWitness` represents the composition
// of two such proofs along an inheritance chain". If `IDerived :
// IBase` and `T : IDerived`, then `T : IBase` is obtained
// transitively; the checker accepts a `T` where `T : IBase` is
// required.

//TEST:INTERPRET(filecheck=CHECK):

interface IBase { int base(); }
interface IDerived : IBase { int derived(); }

struct S : IDerived
{
    int base()    { return 1; }
    int derived() { return 2; }
}

int callBase<T : IBase>(T v) { return v.base(); }

void main()
{
    S s;
    // CHECK: r=1
    printf("r=%d\n", callBase(s));
}
